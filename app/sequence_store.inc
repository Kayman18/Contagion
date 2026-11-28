enum {USER_SEQ_SLOTS=8, USER_SEQ_EVENT=13, USER_SEQ_NAME=10,
      SEQ_CONFIG_BYTES=offsetof(SeqPattern,cells), SEQ_GLOBAL_BYTES=8};
typedef struct {
 char name[USER_SEQ_NAME];
 uint8_t global[SEQ_GLOBAL_BYTES],config[SEQ_MODES][SEQ_LANES][SEQ_CONFIG_BYTES],seed[8];
 uint8_t phrase[320];
} UserSequence;
static struct {
 uint8_t magic[4],occupied;
 UserSequence slot[USER_SEQ_SLOTS];
} sequence_bank;
_Static_assert(SEQ_CONFIG_BYTES==33,"sequence settings layout changed");
_Static_assert(sizeof(sequence_bank)<=PLAT_STORE_MAX,"sequence bank exceeds storage");

static struct {
 SeqPattern pattern[SEQ_MODES][SEQ_LANES];
 uint32_t rng[SEQ_LANES];
 uint8_t global[SEQ_GLOBAL_BYTES];
} sequence_stage;
static volatile unsigned sequence_job;
static unsigned sequence_slot,sequence_active,sequence_pending,sequence_status,sequence_bad;
static char sequence_name[USER_SEQ_NAME+1];
static unsigned sequence_name_edit,sequence_name_pos;

static int sequence_occupied(unsigned slot){
 return (sequence_bank.occupied&(1u<<slot))!=0;
}
static void sequence_load_bank(void){
 sequence_job=sequence_slot=sequence_status=sequence_name_edit=sequence_name_pos=0;
 sequence_pending=sequence_active=USER_SEQ_SLOTS;
 int bytes=plat_store_load(OBJ_USER_SEQUENCES,&sequence_bank,sizeof(sequence_bank));
 sequence_bad=bytes>=0&&(bytes!=sizeof(sequence_bank)||memcmp(sequence_bank.magic,"VCS1",4));
 if(bytes<0){
  memset(&sequence_bank,0,sizeof(sequence_bank));
  memcpy(sequence_bank.magic,"VCS1",4);
 }
}

static void sequence_audio(unsigned restore){
 uint8_t *g=sequence_stage.global;
 if(restore){
  seq_stop();
  memcpy(seq.pattern,sequence_stage.pattern,sizeof(seq.pattern));
  seq.mode=g[0];seq.bpm=g[1];seq.root=g[2];seq.scale=g[3];seq.velocity=g[4];
  seq.enabled[0]=g[5];seq.enabled[1]=g[6];seq.root_octave=(int)g[7]-2;
  for(unsigned i=0;i<SEQ_LANES;i++){
   seq_reset_lane(&seq.lane[i]);seq.lane[i].rng=sequence_stage.rng[i];
  }
  seq_stamp++;
 }else{
  memcpy(sequence_stage.pattern,seq.pattern,sizeof(seq.pattern));
  g[0]=seq.mode;g[1]=seq.bpm;g[2]=seq.root;g[3]=seq.scale;g[4]=seq.velocity;
  g[5]=seq.enabled[0];g[6]=seq.enabled[1];g[7]=seq.root_octave+2;
  for(unsigned i=0;i<SEQ_LANES;i++)sequence_stage.rng[i]=seq.lane[i].rng;
 }
 VC_BARRIER();sequence_job=restore?4:2;
}

/* OXI uses a 7-bit pitch and 7-bit trigger roll. SIG+ uses a 7-bit pitch,
 * 3-bit division and 3 articulation bits. Stored pitch 127 denotes a rest. */
static void sequence_codec(UserSequence *record,int read){
 unsigned bit=0;
 for(unsigned mode=0;mode<SEQ_MODES;mode++)for(unsigned lane=0;lane<SEQ_LANES;lane++){
  SeqPattern *p=&sequence_stage.pattern[mode][lane];
  uint8_t *config=record->config[mode][lane];
  if(read)memcpy(p,config,SEQ_CONFIG_BYTES);
  else memcpy(config,p,SEQ_CONFIG_BYTES);
  unsigned count=mode==SEQ_PATTERN?32:64,width=mode==SEQ_PATTERN?14:13;
  for(unsigned i=0;i<count;i++){
   SeqCell *cell=&p->cells[i];
   unsigned byte=bit>>3,shift=bit&7u,mask=(1u<<width)-1u;
   unsigned packed=(unsigned)record->phrase[byte]|(unsigned)record->phrase[byte+1]<<8;
   if(shift+width>16)packed|=(unsigned)record->phrase[byte+2]<<16;
   if(read){
    unsigned value=(packed>>shift)&mask,pitch=value&127u;
    cell->pitch=pitch==127?SEQ_REST:pitch;
    cell->flags=mode==SEQ_PATTERN?value>>7:(value>>7)&7u;
    cell->ticks=0;
    if(mode==SEQ_EVENT_STREAM){
     unsigned division=value>>10;
     cell->ticks=seq_lengths[division<5?division:0];
    }
   }else{
    unsigned pitch=cell->pitch==SEQ_REST?127:cell->pitch;
    unsigned value=pitch|((unsigned)cell->flags<<7);
    if(mode==SEQ_EVENT_STREAM){
     unsigned division=0;
     while(division<4&&seq_lengths[division]!=cell->ticks)division++;
     value|=division<<10;
    }
    packed=(packed&~(mask<<shift))|((value&mask)<<shift);
    record->phrase[byte]=(uint8_t)packed;record->phrase[byte+1]=(uint8_t)(packed>>8);
    if(shift+width>16)record->phrase[byte+2]=(uint8_t)(packed>>16);
   }
   bit+=width;
  }
 }
 if(read){
  memcpy(sequence_stage.global,record->global,SEQ_GLOBAL_BYTES);
  memcpy(sequence_stage.rng,record->seed,sizeof(record->seed));
 }else{
  memcpy(record->global,sequence_stage.global,SEQ_GLOBAL_BYTES);
  memcpy(record->seed,sequence_stage.rng,sizeof(record->seed));
 }
}

static int sequence_valid(void){
 const uint8_t *g=sequence_stage.global;
 if(g[0]>=SEQ_MODES||g[1]<30||g[1]>240||g[2]>11||g[3]>5||!g[4]||g[4]>127||
    g[5]>1||g[6]>1||g[7]>4)return 0;
 for(unsigned m=0;m<SEQ_MODES;m++)for(unsigned i=0;i<SEQ_LANES;i++){
  const SeqPattern *p=&sequence_stage.pattern[m][i];
  if(!sequence_stage.rng[i]||p->low>p->high||p->high>7||p->division>3||
     p->swing<50||p->swing>75||!p->length||p->length>32||!p->bars||p->bars>4||
     p->filled>(m==SEQ_PATTERN?32:64)||p->gate<10||p->gate>100||p->morph>100)return 0;
 }
 return 1;
}

static void sequence_background(void){
 if(sequence_job==2){
  VC_BARRIER();
  UserSequence *record=&sequence_bank.slot[sequence_slot];
  UserSequence old=*record;
  uint8_t old_mask=sequence_bank.occupied;
  memset(record,0,sizeof(*record));
  memcpy(record->name,sequence_name,USER_SEQ_NAME);
  sequence_codec(record,0);
  sequence_bank.occupied|=1u<<sequence_slot;
  if(plat_store_save(OBJ_USER_SEQUENCES,&sequence_bank,sizeof(sequence_bank))){
   *record=old;sequence_bank.occupied=old_mask;sequence_status=5;
  }else{sequence_status=1;sequence_name_edit=0;sequence_active=sequence_slot;}
  sequence_job=0;redraw=1;
 }else if(sequence_job==4){
  sequence_active=sequence_pending;
  sequence_pending=USER_SEQ_SLOTS;
  sequence_status=2;sequence_name_edit=0;sequence_job=0;redraw=1;
 }else if(!sequence_job&&sequence_pending<USER_SEQ_SLOTS){
  if(sequence_bad||!sequence_occupied(sequence_pending)){
   sequence_status=sequence_bad?6:3;sequence_pending=USER_SEQ_SLOTS;redraw=1;
  }else{
   sequence_codec(&sequence_bank.slot[sequence_pending],1);
   if(!sequence_valid()){
    sequence_status=5;sequence_pending=USER_SEQ_SLOTS;redraw=1;
   }else{
    sequence_job=3;vc_post(USER_SEQ_EVENT,1,0);
   }
  }
 }
}

static void sequence_name_for(unsigned slot,char out[USER_SEQ_NAME+1]){
 if(sequence_occupied(slot))memcpy(out,sequence_bank.slot[slot].name,USER_SEQ_NAME);
 else{
  memset(out,0,USER_SEQ_NAME);
  memcpy(out,"SEQ ",4);
  out[4]=(char)('0'+(slot+1)/10);out[5]=(char)('0'+(slot+1)%10);
 }
 out[USER_SEQ_NAME]=0;
}

static void sequence_input(unsigned knob,int delta){
 if(sequence_job)return;
 if(knob==EN_K1){
  sequence_slot=(unsigned)clamp((int)sequence_slot+delta,0,USER_SEQ_SLOTS-1);
  sequence_status=0;
 }else if(knob==EN_K2)sequence_pending=sequence_slot;
 else if(knob==EN_K3||knob==EN_K4){
  if(!sequence_name_edit){
   sequence_name_for(sequence_slot,sequence_name);
   unsigned n=0;while(sequence_name[n])n++;
   memset(sequence_name+n,' ',USER_SEQ_NAME-n);
   sequence_name_pos=0;sequence_name_edit=1;
  }
  if(knob==EN_K3)sequence_name_pos=(unsigned)clamp((int)sequence_name_pos+delta,0,USER_SEQ_NAME-1);
  else sequence_name[sequence_name_pos]=(char)clamp(sequence_name[sequence_name_pos]+delta,32,126);
 }
 redraw=1;
}

/* Transport, edits and note ownership live on the audio core. The UI sends
 * relative edits through the existing event queue and reads byte-sized fields. */
enum {
 SEQ_EVENT=12, SEQ_RATE=44100, SEQ_LANES=2, SEQ_MODES=2
};
/* Modes are also used as array indices. */
enum {SEQ_PATTERN=0,SEQ_EVENT_STREAM=1,SEQ_CELLS=64,SEQ_REST=255,SEQ_SLIDE=4};
enum {
 SEQ_NOTE_WEIGHT=0,SEQ_DURATION_WEIGHT=12,
 SEQ_LOW=17,SEQ_HIGH,SEQ_DENSITY,SEQ_DIVISION,SEQ_GATE,SEQ_SWING,SEQ_PITCH_EVOLVE,
 SEQ_RHYTHM_EVOLVE,SEQ_LENGTH,SEQ_HOLD,SEQ_REST_CHANCE,SEQ_RATCHET,SEQ_GLIDE,SEQ_BARS,SEQ_MORPH,
 SEQ_TOGGLE=128,SEQ_MODE,SEQ_BPM,SEQ_ROOT,SEQ_SCALE,SEQ_ENABLE_A,SEQ_ENABLE_B,SEQ_VELOCITY,SEQ_OCTAVE
};
static const char *const seq_scale_names[]={
 "CHROMATIC","MAJOR","MINOR","DORIAN","MAJ PENTA","MIN PENTA"
};
static const char *const seq_note_names[]={
 "C","C#","D","EB","E","F","F#","G","AB","A","BB","B"
};
static const uint16_t seq_scale_masks[]={0xfff,0xab5,0x5ad,0x6ad,0x295,0x4a9};
static const uint8_t seq_lengths[]={2,4,8,16,32}; /* 1/32 notes */
static const uint8_t seq_divisions[]={8,4,2,1};
static const char *const seq_duration_names[]={"1/16","1/8","1/4","1/2","1 BAR"};
static const char *const seq_division_names[]={"1/4","1/8","1/16","1/32"};

typedef struct {
 uint8_t pitch, ticks, flags;
 /* In stored OXI cells, flags holds a 0–99 trigger roll. Playback cells
  * store pulse count in bits 0..1 and a slide request in bit 2. */
} SeqCell;
typedef struct {
 uint8_t weights[12], duration[5], low, high, density, division, gate, swing;
 uint8_t pitch_evolve, rhythm_evolve, length, hold, rest, ratchet, glide, bars;
 uint8_t filled,morph;
 SeqCell cells[SEQ_CELLS];
} SeqPattern;
typedef struct {
 uint32_t rng;
 unsigned left, gate_left, pulse_left, remainder;
 uint8_t position, next, elapsed, pulses, gate_percent;
 int note, previous;
 SeqCell cell;
} SeqLane;

static struct {
 SeqPattern pattern[SEQ_MODES][SEQ_LANES];
 SeqLane lane[SEQ_LANES];
 uint8_t mode, running, enabled[SEQ_LANES], bpm, root, scale, velocity;
 int8_t root_octave;
 uint8_t trim_fusion_tail;
 uint32_t manual[2][4];
} seq;
static unsigned seq_ui_lane, seq_note_cursor[SEQ_LANES], seq_duration_cursor[SEQ_LANES];
static unsigned seq_ui_stamp;
static volatile unsigned seq_stamp;
/* One aligned event word per lane preserves brief gates for the UI. */
static volatile uint32_t seq_key_flash[SEQ_LANES];
static uint32_t seq_seen_flash[SEQ_LANES],seq_flash_time[SEQ_LANES];
static int seq_live_key[SEQ_LANES],seq_drawn_key[SEQ_LANES];

static void seq_init(void){
 static const uint8_t notes[2][12]={
  {100,0,20,64,0,12,0,85,8,0,26,0},{55,0,25,92,0,12,0,72,19,0,48,0}
 };
 static const uint8_t durations[2][5]={{10,30,65,85,20},{45,80,35,15,5}};
 memset(&seq,0,sizeof(seq));
 for(unsigned lane=0;lane<SEQ_LANES;lane++){
  for(unsigned mode=0;mode<SEQ_MODES;mode++){
   SeqPattern *p=&seq.pattern[mode][lane];
   memcpy(p->weights,notes[lane],sizeof(p->weights));
   memcpy(p->duration,durations[lane],sizeof(p->duration));
   p->low=2+2*lane;p->high=3+2*lane;
   p->density=lane?48:68;p->division=2;p->gate=mode==SEQ_PATTERN?55:70;p->swing=50;
   p->pitch_evolve=lane?35:15;p->rhythm_evolve=lane?30:20;p->length=lane?12:16;
   p->rest=lane?25:12;p->ratchet=lane?18:4;p->glide=lane?8:4;p->bars=2;
  }
  seq.lane[lane].rng=0xc074a613u^(lane*0x9e3779b9u);
  seq.lane[lane].note=seq.lane[lane].previous=-1;
  seq.enabled[lane]=1;seq_note_cursor[lane]=seq_duration_cursor[lane]=0;
  seq_key_flash[lane]=seq_seen_flash[lane]=seq_flash_time[lane]=0;
  seq_live_key[lane]=seq_drawn_key[lane]=-1;
 }
 seq.bpm=120;seq.scale=2;seq.velocity=100;
 seq_ui_lane=seq_ui_stamp=seq_stamp=0;
}

static unsigned seq_random(SeqLane *lane,unsigned range){
 uint32_t x=lane->rng;
 x^=x<<13;x^=x>>17;x^=x<<5;lane->rng=x;
 return x%range;
}

static unsigned seq_weighted(SeqLane *lane,const uint8_t *weights,unsigned count,unsigned mask){
 unsigned total=0;
 for(unsigned i=0;i<count;i++)if(mask&(1u<<i))total+=weights[i];
 if(!total)return count;
 unsigned pick=seq_random(lane,total);
 for(unsigned i=0;i<count;i++)if(mask&(1u<<i)){
  if(pick<weights[i])return i;
  pick-=weights[i];
 }
 return count;
}

static unsigned seq_morph_degree(SeqLane *lane,const SeqPattern *p,unsigned division){
 static const uint8_t accents[]={25,60,100,155,230};
 unsigned mask=seq_scale_masks[seq.scale],total=p->weights[0];
 for(unsigned note=1;note<12;note++)if(mask&(1u<<note))total+=p->weights[note];
 if(!total)return 12;
 /* Use the same root roll as the original picker; only redistribute the
  * non-root share. Hash that draw separately so rhythm RNG stays in sync. */
 if(seq_random(lane,total)<p->weights[0])return 0;
 unsigned phase=(division+lane->next)%5u,sum=0;
 uint16_t weights[12]={0};
 for(unsigned note=1;note<12;note++)if((mask&(1u<<note))&&p->weights[note]){
  unsigned factor=(100u-p->morph)*100u+p->morph*accents[(phase+note)%5u];
  weights[note]=(uint16_t)(p->weights[note]*factor/100u);
  sum+=weights[note];
 }
 uint32_t cross=lane->rng*0x9e3779b9u;
 unsigned pick=(cross^(cross>>16))%sum;
 for(unsigned note=1;note<12;note++){
  if(pick<weights[note])return note;
  pick-=weights[note];
 }
 return 12;
}

static unsigned seq_pick_pitch(SeqLane *lane,const SeqPattern *p,unsigned division){
 unsigned note=p->morph?seq_morph_degree(lane,p,division):
  seq_weighted(lane,p->weights,12,seq_scale_masks[seq.scale]);
 if(note==12)return SEQ_REST;
 unsigned octave=p->low+seq_random(lane,p->high-p->low+1u);
 return 12u*(octave+1u)+seq.root+note;
}

static int seq_manual_held(unsigned note){
 return ((seq.manual[0][note>>5]|seq.manual[1][note>>5])>>(note&31u))&1u;
}

/* USB MIDI channels 1 (keys), 2 (lane A), and 3 (lane B). */
static void seq_midi_note(unsigned channel,unsigned note,unsigned velocity){
 unsigned status=(velocity?0x90u:0x80u)|channel;
 plat_midi_out((status>>4)|(status<<8)|(note<<16)|(velocity<<24));
}

static void seq_release_lane(unsigned index){
 SeqLane *lane=&seq.lane[index];
 int note=lane->note;
 if(note<0)return;
 seq_midi_note(index+1u,(unsigned)note,0);
 lane->note=-1;
 if(seq.lane[index^1u].note!=note&&!seq_manual_held((unsigned)note)){
  unsigned limit=127;
  /* A sounding FusionBrs can overlap the next two unison groups at long
   * gates. Shorten its generated tail gradually above 75% gate. */
  if(seq.trim_fusion_tail&&!lane->gate_left&&lane->gate_percent>75)
   limit=6u+(100u-lane->gate_percent)*14u/25u;
  vc_generated_note_off(&synth,note,limit);
 }
}

static void seq_manual(unsigned source,unsigned note,unsigned velocity,unsigned release_velocity){
 if(note>=128)return;
 uint32_t mask=1u<<(note&31u);
 if(source==0&&(velocity||(seq.manual[0][note>>5]&mask)))
  seq_midi_note(0,note,velocity);
 if(velocity){
  seq.manual[source][note>>5]|=mask;
  for(unsigned i=0;i<SEQ_LANES;i++)if(seq.lane[i].note==(int)note)seq_release_lane(i);
  vc_note_on(&synth,note,velocity);
 }else{
  seq.manual[source][note>>5]&=~mask;
  if(!seq_manual_held(note)&&seq.lane[0].note!=(int)note&&seq.lane[1].note!=(int)note)
   vc_note_off_velocity(&synth,note,release_velocity);
 }
}

static void seq_reset_lane(SeqLane *lane){
 lane->left=lane->gate_left=lane->pulse_left=lane->remainder=0;
 lane->position=lane->next=lane->elapsed=lane->pulses=0;
 lane->note=lane->previous=-1;
}

static void seq_stop(void){
 if(!seq.running)return;
 for(unsigned i=0;i<SEQ_LANES;i++){
  seq_release_lane(i);
  seq_reset_lane(&seq.lane[i]);
 }
 seq.running=0;seq_stamp++;
}

static void seq_silence(void){
 for(unsigned word=0;word<4;word++){
  uint32_t held=seq.manual[0][word];
  for(unsigned bit=0;held;held>>=1,bit++)
   if(held&1u)seq_midi_note(0,32u*word+bit,0);
 }
 for(unsigned i=0;i<SEQ_LANES;i++){
  if(seq.lane[i].note>=0)seq_midi_note(i+1u,(unsigned)seq.lane[i].note,0);
  seq.lane[i].note=seq.lane[i].previous=-1;
  seq.lane[i].gate_left=0;
 }
 memset(seq.manual,0,sizeof(seq.manual));
}

static void seq_mute(void){
 seq_stop();
 seq_silence();
}

static void seq_edit_pattern(unsigned control,int delta){
 unsigned index=control/64;
 SeqPattern *p=&seq.pattern[seq.mode][index];
 control%=64;
 if(control<SEQ_DURATION_WEIGHT){
  if(seq_scale_masks[seq.scale]&(1u<<control))
   p->weights[control]=(uint8_t)clamp(p->weights[control]+delta,0,100);
  return;
 }
 if(control<SEQ_LOW){
  unsigned i=control-SEQ_DURATION_WEIGHT;
  p->duration[i]=(uint8_t)clamp(p->duration[i]+delta,0,100);
  return;
 }
 uint8_t *field;
 int low=0,high=100;
 switch(control){
 case SEQ_LOW:field=&p->low;low=0;high=p->high;break;
 case SEQ_HIGH:field=&p->high;low=p->low;high=7;break;
 case SEQ_DENSITY:field=&p->density;break;
 case SEQ_DIVISION:field=&p->division;high=3;break;
 case SEQ_GATE:field=&p->gate;low=10;break;
 case SEQ_SWING:field=&p->swing;low=50;high=75;break;
 case SEQ_PITCH_EVOLVE:field=&p->pitch_evolve;break;
 case SEQ_RHYTHM_EVOLVE:field=&p->rhythm_evolve;break;
 case SEQ_LENGTH:field=&p->length;low=1;high=32;break;
 case SEQ_HOLD:field=&p->hold;high=1;break;
 case SEQ_REST_CHANCE:field=&p->rest;break;
 case SEQ_RATCHET:field=&p->ratchet;break;
 case SEQ_GLIDE:field=&p->glide;break;
 case SEQ_BARS:field=&p->bars;low=1;high=4;break;
 case SEQ_MORPH:if(seq.mode!=SEQ_EVENT_STREAM)return;field=&p->morph;break;
 default:return;
 }
 unsigned value=(unsigned)clamp(*field+delta,low,high);
 if(value==*field)return;
 *field=(uint8_t)value;
 if(control==SEQ_BARS){
  /* Keep the current event intact. Build a phrase of the new size at wrap. */
  p->filled=0;
  seq.lane[index].elapsed=p->bars*32u;
 }
}

static unsigned seq_scale_notes(unsigned scale,uint8_t notes[12]){
 unsigned mask=seq_scale_masks[scale],count=0;
 for(unsigned i=0;i<12;i++)if(mask&(1u<<i))notes[count++]=(uint8_t)i;
 return count;
}

static unsigned seq_note_index(unsigned lane,unsigned count){
 return (unsigned)clamp((int)seq_note_cursor[lane],0,(int)count-1);
}

static void seq_set_tempo(unsigned bpm){
 unsigned old=seq.bpm;
 if(bpm==old)return;
 for(unsigned i=0;i<SEQ_LANES;i++){
  SeqLane *lane=&seq.lane[i];
  lane->left=(lane->left*old+bpm-1)/bpm;
  lane->gate_left=(lane->gate_left*old+bpm-1)/bpm;
  lane->pulse_left=(lane->pulse_left*old+bpm-1)/bpm;
  lane->remainder=lane->remainder*bpm/old;
 }
 seq.bpm=(uint8_t)bpm;
}

static void seq_event(unsigned control,unsigned value){
 int delta=(int)value-128;
 if(control<SEQ_TOGGLE)seq_edit_pattern(control,delta);
 else switch(control){
 case SEQ_TOGGLE:
  if(seq.running){seq_stop();return;}
  if(seq.mode==SEQ_EVENT_STREAM)for(unsigned i=0;i<SEQ_LANES;i++){
   SeqPattern *p=&seq.pattern[SEQ_EVENT_STREAM][i];
   if(!p->hold)p->filled=0;
  }
  seq.running=1;
  break;
 case SEQ_MODE:{
  unsigned mode=(unsigned)clamp(seq.mode+delta,0,SEQ_MODES-1);
  if(mode!=seq.mode){seq_stop();seq.mode=(uint8_t)mode;}
  break;
 }
 case SEQ_BPM:
  seq_set_tempo((unsigned)clamp(seq.bpm+delta,30,240));
  break;
 case SEQ_OCTAVE:seq.root_octave=(int8_t)clamp(seq.root_octave+delta,-2,2);break;
 case SEQ_ROOT:seq.root=(uint8_t)clamp(seq.root+delta,0,11);break;
 case SEQ_SCALE:seq.scale=(uint8_t)clamp(seq.scale+delta,0,5);break;
 case SEQ_ENABLE_A:case SEQ_ENABLE_B:{
  unsigned i=control-SEQ_ENABLE_A;
  seq.enabled[i]=(uint8_t)clamp(seq.enabled[i]+delta,0,1);
  if(!seq.enabled[i])seq_release_lane(i);
  break;
 }
 case SEQ_VELOCITY:seq.velocity=(uint8_t)clamp(seq.velocity+delta,1,127);break;
 default:return;
 }
 seq_stamp++;
}

static int seq_oxi_gate(const SeqPattern *p,unsigned step){
 unsigned density=p->density;
 return density==100||(step<p->filled&&p->cells[step].flags<density);
}

static SeqCell seq_oxi_cell(SeqLane *lane,SeqPattern *p){
 unsigned step=lane->next<p->length?lane->next:0;
 SeqCell *cell=&p->cells[step];
 int fresh=step>=p->filled;
 if(fresh||(!p->hold&&p->pitch_evolve&&seq_random(lane,100)<p->pitch_evolve))
  cell->pitch=(uint8_t)seq_pick_pitch(lane,p,0);
 if(fresh||(!p->hold&&p->rhythm_evolve&&seq_random(lane,100)<p->rhythm_evolve))
  cell->flags=(uint8_t)seq_random(lane,100);
 if(fresh)p->filled=step+1u;
 lane->position=step;lane->next=step+1u;
 SeqCell play=*cell;
 play.ticks=seq_divisions[p->division];
 play.flags=(uint8_t)seq_oxi_gate(p,step);
 return play;
}

static SeqCell seq_sig_cell(SeqLane *lane,SeqPattern *p){
 unsigned total=32u*p->bars;
 if(lane->elapsed>=total){
  lane->elapsed=0;lane->next=0;
  if(!p->hold)p->filled=0;
 }
 unsigned step=lane->next;
 SeqCell *cell=&p->cells[step];
 if(step>=p->filled){
  unsigned length=seq_weighted(lane,p->duration,5,31);
  if(length==5)length=0;
  cell->ticks=seq_lengths[length];
  cell->pitch=seq_random(lane,100)<p->rest?SEQ_REST:(uint8_t)seq_pick_pitch(lane,p,length);
  cell->flags=1;
  if(cell->pitch!=SEQ_REST){
   if(seq_random(lane,100)<p->ratchet)cell->flags=2+(seq_random(lane,100)<25);
   if(seq_random(lane,100)<p->glide)cell->flags|=SEQ_SLIDE;
  }
  p->filled=step+1u;
 }
 SeqCell play=*cell;
 if(play.ticks>total-lane->elapsed)play.ticks=total-lane->elapsed;
 lane->position=lane->elapsed;lane->elapsed+=play.ticks;lane->next=step+1u;
 return play;
}

static unsigned seq_frames(SeqLane *lane,const SeqPattern *p,unsigned ticks){
 /* Carry the fractional sample on each lane, so independent divisions stay
  * on one tempo grid. The last step of an odd OXI loop stays straight. */
 unsigned divisor=8u*seq.bpm;
 unsigned numerator=ticks*(SEQ_RATE*60u);
 if(seq.mode==SEQ_PATTERN){
  unsigned tilt=lane->position&1u?100u-p->swing:p->swing;
  if((p->length&1u)&&lane->position+1u==p->length)tilt=50;
  divisor*=50u;numerator*=tilt;
 }
 numerator+=lane->remainder;
 unsigned frames=numerator/divisor;
 lane->remainder=numerator-frames*divisor;
 return frames;
}

static void seq_pulse(unsigned index,const SeqPattern *p){
 SeqLane *lane=&seq.lane[index];
 unsigned note=lane->cell.pitch;
 unsigned span=lane->pulse_left=lane->left/lane->pulses;
 unsigned slide=(lane->cell.flags&SEQ_SLIDE)&&lane->pulses==(lane->cell.flags&3u);
 lane->pulses--;
 if(!seq.enabled[index]||seq_manual_held(note)){lane->previous=-1;return;}
 if(seq.lane[index^1u].note!=(int)note){
  if(slide&&lane->previous>=0&&lane->previous!=(int)note)
   vc_generated_slide_on(&synth,note,seq.velocity,lane->previous);
  else vc_note_on(&synth,note,seq.velocity);
 }
 seq_midi_note(index+1u,note,seq.velocity);
 lane->note=lane->previous=(int)note;
 seq_key_flash[index]=((seq_key_flash[index]+128u)&~127u)|note;
 lane->gate_percent=p->gate;
 lane->gate_left=span*p->gate/100u;
}

static uint8_t seq_transpose_octave(unsigned pitch){
 if(pitch==SEQ_REST)return SEQ_REST;
 int note=(int)pitch+12*seq.root_octave;
 /* Preserve the scale degree if a register extreme crosses the MIDI range. */
 while(note<0)note+=12;
 while(note>127)note-=12;
 return (uint8_t)note;
}

static void seq_advance(unsigned index){
 SeqLane *lane=&seq.lane[index];
 SeqPattern *p=&seq.pattern[seq.mode][index];
 if(!lane->left){
  lane->cell=seq.mode==SEQ_PATTERN?seq_oxi_cell(lane,p):seq_sig_cell(lane,p);
  lane->cell.pitch=seq_transpose_octave(lane->cell.pitch);
  lane->left=seq_frames(lane,p,lane->cell.ticks);
  lane->pulses=lane->cell.pitch==SEQ_REST?0:lane->cell.flags&3u;
  if(!lane->pulses)lane->previous=-1;
  lane->pulse_left=0;
 }
 if(!lane->pulse_left&&lane->pulses)seq_pulse(index,p);
}

static unsigned seq_render_span(unsigned limit){
 if(!seq.running)return limit;
 /* Release both lanes before either starts a new pulse. This also avoids
  * an order-dependent cut when they share a note in adjacent events. */
 for(unsigned i=0;i<SEQ_LANES;i++)if(!seq.lane[i].gate_left)seq_release_lane(i);
 for(unsigned i=0;i<SEQ_LANES;i++){
  SeqLane *lane=&seq.lane[i];
  if(!lane->left||(!lane->pulse_left&&lane->pulses))seq_advance(i);
  if(lane->left<limit)limit=lane->left;
  if(lane->pulses&&lane->pulse_left<limit)limit=lane->pulse_left;
  if(lane->gate_left&&lane->gate_left<limit)limit=lane->gate_left;
 }
 for(unsigned i=0;i<SEQ_LANES;i++){
  SeqLane *lane=&seq.lane[i];
  lane->left-=limit;
  if(lane->pulses)lane->pulse_left-=limit;
  if(lane->gate_left)lane->gate_left-=limit;
 }
 return limit;
}

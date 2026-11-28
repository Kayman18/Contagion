#define SEQ_BAR_OFF RGB(43,34,34)
#define SEQ_BAR_ON RGB(137,62,47)

typedef struct {const char *label;char value[20];} SeqReadout;

static void access_seq_right(int y,const char *s,uint16_t color){
 cv_text(232-text_w(&FONT_XS,s),y,&FONT_XS,s,color);
}
static void access_seq_number(SeqReadout *cell,const char *label,unsigned value,const char *unit){
 cell->label=label;
 vc_num(cell->value,value);
 char *end=cell->value;
 while(*end)end++;
 vc_copy(end,unit);
}
static void access_seq_text(SeqReadout *cell,const char *label,const char *value){
 cell->label=label;vc_copy(cell->value,value);
}

/* The 76-pixel chart is drawn in two strips to fit the existing canvas. */
static void access_seq_durations(const SeqPattern *p,unsigned selected){
 const int width=44;
 for(int strip=0;strip<2;strip++){
  int y=strip*38;
  cv_begin(240,38,ACCESS_BG);
  for(unsigned i=0;i<5;i++){
   unsigned weight=p->duration[i];
   int height=(weight*60+99)/100,x=8+i*width;
   cv_rect(x+2,2-y,width-5,60,SEQ_BAR_OFF);
   if(height)cv_rect(x+2,62-height-y,width-5,height,i==selected?ACCESS_AMBER:SEQ_BAR_ON);
   if(strip){
    const char *name=seq_duration_names[i];
    cv_text(x+(width-text_w(&FONT_XS,name))/2,62-y,&FONT_XS,name,
            i==selected?ACCESS_AMBER:ACCESS_SUB);
   }
  }
  cv_blit(0,54+y);
 }
}

/* Six LEDs per key. The top lit LED is dimmed for fractional levels. */
static void access_seq_key_meter(int x,int bottom,int width,unsigned weight,int selected){
 cv_rect(x-2,bottom-25,width+4,27,RGB(28,23,22));
 for(int segment=0;segment<6;segment++){
  unsigned level=(unsigned)clamp((int)weight*6-segment*100,0,100);
  unsigned r=58u+level*(selected?192u:174u)/100u;
  unsigned g=45u+level*(selected?131u:70u)/100u;
  unsigned b=40u+level*(selected?4u:0u)/100u;
  cv_rect(x,bottom-2-4*segment,width,3,RGB(r,g,b));
 }
}

static uint16_t access_seq_key_color(unsigned pitch,uint16_t base){
 if(seq_live_key[seq_ui_lane]==(int)pitch)return RGB(204,133,39);
 if(seq_live_key[seq_ui_lane^1u]==(int)pitch)return RGB(156,65,56);
 return base;
}

static void access_seq_keyboard(const SeqPattern *p,unsigned scale,unsigned selected){
 static const uint8_t white[]={0,2,4,5,7,9,11};
 static const uint8_t black[]={1,3,6,8,10};
 static const uint8_t boundary[]={1,2,4,5,6};
 unsigned root=seq.root,mask=seq_scale_masks[scale];
 for(int strip=0;strip<2;strip++){
  int y=strip*38;
  cv_begin(240,38,ACCESS_BG);
  for(unsigned i=0;i<7;i++){
   unsigned degree=(white[i]+12u-root)%12u;
   int x=8+32*i,active=(mask&(1u<<degree))!=0,chosen=selected==degree;
   cv_rect(x,1-y,31,72,chosen?ACCESS_AMBER:RGB(17,15,15));
   cv_rect(x+1,2-y,29,69,access_seq_key_color(white[i],active?RGB(225,212,191):RGB(87,78,72)));
   access_seq_key_meter(x+9,53-y,13,active?p->weights[degree]:0,chosen);
   if(strip){
    const char *name=seq_note_names[white[i]];
    cv_text(x+15-text_w(&FONT_XS,name)/2,55-y,&FONT_XS,name,
            active?RGB(45,33,28):RGB(164,150,139));
   }
  }
  if(!strip)for(unsigned i=0;i<5;i++){
   unsigned degree=(black[i]+12u-root)%12u;
   int x=8+32*boundary[i],active=(mask&(1u<<degree))!=0,chosen=selected==degree;
   cv_rect(x-10,0,20,37,chosen?ACCESS_AMBER:RGB(17,15,15));
   cv_rect(x-9,1,18,35,access_seq_key_color(black[i],active?RGB(48,39,34):RGB(31,29,28)));
   access_seq_key_meter(x-4,31,9,active?p->weights[degree]:0,chosen);
  }
  cv_blit(0,54+y);
 }
 for(unsigned i=0;i<SEQ_LANES;i++)seq_drawn_key[i]=seq_live_key[i];
}

static void access_seq_triggers(const SeqPattern *p,const SeqLane *lane){
 unsigned length=p->length;
 int width=224/(int)length;
 for(int strip=0;strip<2;strip++){
  int y=strip*38;
  cv_begin(240,38,ACCESS_BG);
  for(unsigned i=0;i<length;i++){
   int x=8+i*width;
   unsigned playing=seq.running&&i==lane->position;
   unsigned active=seq_oxi_gate(p,i);
   cv_rect(x+1,2-y,width-2,60,active?(playing?ACCESS_AMBER:SEQ_BAR_ON):SEQ_BAR_OFF);
   if(playing&&strip)cv_rect(x+1,65-y,width-2,2,ACCESS_AMBER);
  }
  cv_blit(0,54+y);
 }
}

static void access_seq_root(char label[20]){
 const char *note=seq_note_names[seq.root];
 int offset=seq.root_octave;
 vc_copy(label,note);
 char *end=label;
 while(*end)end++;
 vc_copy(end,"  OCT ");
 end+=6;*end++=offset<0?'-':'+';
 vc_num(end,(unsigned)(offset<0?-offset:offset));
}

static void access_seq_phrase(const SeqPattern *p,const SeqLane *lane){
 unsigned bars=p->bars,total=32u*bars;
 unsigned low=p->low,high=p->high;
 unsigned lowest=12u*(low+1u)+seq.root;
 unsigned range=11u+12u*(high>low?high-low:0);
 unsigned filled=p->filled,position=lane->position,running=seq.running;
 for(int strip=0;strip<2;strip++){
  int top=strip*38;
  cv_begin(240,38,ACCESS_BG);
  for(unsigned bar=0;bar<=bars;bar++)
   cv_rect(8+bar*224u/bars,2-top,1,60,ACCESS_RULE);
  for(int row=2;row<=62;row+=20)cv_rect(8,row-top,225,1,ACCESS_RULE);
  unsigned at=0;
  for(unsigned i=0;i<filled&&at<total;i++){
   SeqCell cell=p->cells[i];
   unsigned end=at+cell.ticks;
   if(end>total)end=total;
   unsigned pulses=cell.flags&3u;
   if(cell.pitch!=SEQ_REST&&pulses){
    unsigned degree=(unsigned)clamp((int)cell.pitch-(int)lowest,0,(int)range);
    int y=57-(int)(degree*53u/range)-top;
    uint16_t color=at==position&&running?ACCESS_AMBER:SEQ_BAR_ON;
    for(unsigned pulse=0;pulse<pulses;pulse++){
     unsigned x=8+(at*pulses+pulse*(end-at))*224u/(total*pulses);
     unsigned right=8+(at*pulses+(pulse+1)*(end-at))*224u/(total*pulses);
     if(right>x)cv_rect(x,y,right-x>2?right-x-1:1,4,color);
    }
    if(cell.flags&SEQ_SLIDE)cv_rect(8+at*224u/total,y+4,2,2,color);
   }
   at=end;
  }
  if(strip&&running&&position<total)cv_rect(8+position*224u/total,65-top,2,5,ACCESS_AMBER);
  cv_blit(0,54+top);
 }
}

static void access_seq_readouts(const SeqReadout *cells,int changed){
 static SeqReadout last[4];
 for(unsigned i=0;i<4;i++){
  if(!changed&&cells[i].label==last[i].label&&!memcmp(cells[i].value,last[i].value,sizeof(last[i].value)))continue;
  unsigned left=i&1u, top=i/2;
  cv_begin(120,44,RGB(30,24,24));
  cv_rect(0,0,120,1,ACCESS_RULE);
  if(left)cv_rect(0,1,1,43,ACCESS_RULE);
  char label[24]="K1 ";
  label[1]=(char)('1'+i);
  vc_copy(label+(page==ACCESS_SEQ_MORPH&&i?0:3),cells[i].label);
  cv_text(8,4,&FONT_XS,label,ACCESS_SUB);
  cv_text(8,19,&FONT_B,cells[i].value,ACCESS_LABEL);
  cv_blit(left*120,130+top*44);
  last[i]=cells[i];
 }
}

static void access_draw_generative(int changed){
 unsigned mode=seq.mode,index=seq_ui_lane;
 const SeqPattern *p=&seq.pattern[mode][index];
 const SeqLane *lane=&seq.lane[index];
 const char *badge=index?"LANE B":"LANE A";
 const char *title,*next,*context,*status;
 const char *transport=seq.running?"RUNNING":"STOPPED";
 unsigned scale=seq.scale,duration=seq_duration_cursor[index];
 uint8_t notes[12];
 unsigned count=seq_scale_notes(scale,notes),selected=seq_note_index(index,count);
 char root[20];
 access_seq_root(root);
 SeqReadout cells[4]={{0}};
 if(page==ACCESS_SEQ_NOTES){
  title="NOTE WEIGHTS";next=mode==SEQ_PATTERN?"SEQ RHYTHM":"SEQ DURATIONS";
  context=seq_scale_names[scale];status=seq_note_names[seq.root];
  access_seq_text(cells,"NOTE",seq_note_names[(seq.root+notes[selected])%12u]);
  access_seq_number(cells+1,"WEIGHT",p->weights[notes[selected]],"%");
  access_seq_number(cells+2,"LOW OCT",p->low,"");
  access_seq_number(cells+3,"HIGH OCT",p->high,"");
 }else if(page==ACCESS_SEQ_RHYTHM&&mode==SEQ_PATTERN){
  title="RHYTHM";next="SEQ EVOLVE";context="TRIGGER PATTERN";status=transport;
  access_seq_number(cells,"DENSITY",p->density,"%");
  access_seq_text(cells+1,"DIVISION",seq_division_names[p->division]);
  access_seq_number(cells+2,"GATE",p->gate,"%");
  access_seq_number(cells+3,"SWING",p->swing,"%");
 }else if(page==ACCESS_SEQ_RHYTHM){
  title="DURATIONS";next="SEQ ARTICULATION";context="TIME BETWEEN EVENTS";status=seq_duration_names[duration];
  access_seq_text(cells,"DURATION",status);
  access_seq_number(cells+1,"WEIGHT",p->duration[duration],"%");
  access_seq_number(cells+2,"REST CHANCE",p->rest,"%");
  access_seq_number(cells+3,"LOOP SPAN",p->bars,p->bars==1?" BAR":" BARS");
 }else if(page==ACCESS_SEQ_SHAPE){
  title=mode==SEQ_PATTERN?"EVOLVE":"ARTICULATION";next="SEQ SETUP";
  context=mode==SEQ_PATTERN?"PHRASE":"EVENT PREVIEW";status=transport;
  if(mode==SEQ_PATTERN){
   access_seq_number(cells,"PITCH EVOLVE",p->pitch_evolve,"%");
   access_seq_number(cells+1,"RHYTHM EVOLVE",p->rhythm_evolve,"%");
   access_seq_number(cells+2,"LENGTH",p->length,p->length==1?" STEP":" STEPS");
  }else{
   access_seq_number(cells,"GATE",p->gate,"%");
   access_seq_number(cells+1,"RATCHET CHANCE",p->ratchet,"%");
   access_seq_number(cells+2,"SLIDE CHANCE",p->glide,"%");
  }
  access_seq_text(cells+3,"PHRASE",p->hold?"HOLD":"LIVE");
 }else if(page==ACCESS_SEQ_SETUP){
  title="SETUP";next="SEQ MODE";context="SHARED CLOCK AND SCALE";status=transport;
  access_seq_text(cells,"EDIT LANE",index?"B":"A");
  access_seq_number(cells+1,"TEMPO",seq.bpm," BPM");
  access_seq_text(cells+2,"ROOT",root);
  access_seq_text(cells+3,"SCALE",seq_scale_names[seq.scale]);
 }else if(page==ACCESS_SEQ_MORPH){
  title="MORPH";next="SEQ NOTES";context="DURATION-BASED NOTE WEIGHTS";status=transport;
  if(p->morph)access_seq_number(cells,"MORPH",p->morph,"%");
  else access_seq_text(cells,"MORPH","OFF");
  access_seq_text(cells+1,"ROOT",root);
  unsigned division=0;
  while(division<5&&seq_lengths[division]!=lane->cell.ticks)division++;
  if(!seq.running)access_seq_text(cells+2,"EVENT DIVISION","--");
  else if(division<5)access_seq_text(cells+2,"EVENT DIVISION",seq_duration_names[division]);
  else access_seq_number(cells+2,"EVENT DIVISION",lane->cell.ticks," TICKS");
  access_seq_text(cells+3,"PHRASE",p->hold?"HOLD":"LIVE");
 }else{
  title="MODE AND LANES";next=mode==SEQ_PATTERN?"SEQ NOTES":"SEQ MORPH";context="GENERATIVE SEQUENCER";status=transport;
  access_seq_text(cells,"MODE",mode==SEQ_PATTERN?"OXI STYLE":"SIG+ STYLE");
  access_seq_text(cells+1,"LANE A",seq.enabled[0]?"ON":"OFF");
  access_seq_text(cells+2,"LANE B",seq.enabled[1]?"ON":"OFF");
  access_seq_number(cells+3,"VELOCITY",seq.velocity,"");
 }
 static const char *last_title,*last_badge,*last_context,*last_status,*last_next;
 static unsigned last_running;
 if(changed||title!=last_title||badge!=last_badge){
  cv_begin(240,32,ACCESS_RED);
  cv_text(8,2,&FONT_M,title,ACCESS_LABEL);
  access_seq_right(8,badge,ACCESS_LABEL);
  cv_rect(0,31,240,1,ACCESS_RULE);
  cv_blit(0,0);
  last_title=title;last_badge=badge;
 }
 if(changed||context!=last_context||status!=last_status||page==ACCESS_SEQ_NOTES){
  cv_begin(240,22,ACCESS_BG);
  cv_text(8,3,&FONT_XS,context,ACCESS_SUB);
  access_seq_right(3,page==ACCESS_SEQ_NOTES?root:status,ACCESS_AMBER);
  cv_blit(0,32);
  last_context=context;last_status=status;
 }
 if(page==ACCESS_SEQ_NOTES)access_seq_keyboard(p,scale,notes[selected]);
 else if(page==ACCESS_SEQ_RHYTHM){
  if(mode==SEQ_PATTERN)access_seq_triggers(p,lane);
  else access_seq_durations(p,duration);
 }else if(page==ACCESS_SEQ_SHAPE&&mode==SEQ_EVENT_STREAM)access_seq_phrase(p,lane);
 else{
  cv_begin(240,42,ACCESS_BG);
  if(page==ACCESS_SEQ_SHAPE){
   cv_text(8,0,&FONT_XS,"PITCH",ACCESS_SUB);
   access_seq_right(0,p->hold||!p->pitch_evolve?"LOCKED":"EVOLVING",ACCESS_LABEL);
   cv_text(8,20,&FONT_XS,"RHYTHM",ACCESS_SUB);
   access_seq_right(20,p->hold||!p->rhythm_evolve?"LOCKED":"EVOLVING",ACCESS_LABEL);
  }else if(page==ACCESS_SEQ_MORPH){
   cv_rect(8,11,224,13,SEQ_BAR_OFF);
   cv_rect(8,11,224u*p->morph/100u,13,ACCESS_AMBER);
  }else if(page==ACCESS_SEQ_MODE){
   access_center(240,1,&FONT_B,mode==SEQ_PATTERN?"OXI STYLE":"SIG+ STYLE",ACCESS_AMBER);
  }
  cv_blit(0,54);
  if(changed){
   cv_begin(240,34,ACCESS_BG);
   cv_blit(0,96);
  }
 }
 access_seq_readouts(cells,changed);
 if(changed||next!=last_next||seq.running!=last_running){
  cv_begin(240,22,ACCESS_BG);
  cv_rect(0,0,240,1,ACCESS_RULE);
  cv_text(8,4,&FONT_XS,next,ACCESS_LABEL);
  access_seq_right(4,seq.running?"PLAY STOP":"PLAY START",ACCESS_AMBER);
  cv_blit(0,218);
  last_next=next;last_running=seq.running;
 }
}

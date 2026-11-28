#include "access_art.inc"
#include "../runtime/wavetable_bank.h"

#define ACCESS_BG RGB(16,16,16)
#define ACCESS_RED RGB(77,15,22)
#define ACCESS_RULE RGB(47,44,44)
#define ACCESS_LABEL RGB(248,236,218)
#define ACCESS_SUB RGB(172,161,151)
#define ACCESS_AMBER RGB(250,176,44)

static void access_center(int width,int y,const felucca_font_t *font,const char *text,uint16_t color){
 int size=text_w(font,text);
 if(size>width-8&&font==&FONT_M){font=&FONT_B;size=text_w(font,text);}
 if(size>width-8){font=&FONT_T;size=text_w(font,text);}
 cv_text((width-size)/2,y,font,text,color);
}

static void access_splash_progress(unsigned percent){
 const uint16_t navy=RGB(4,13,25),cyan=RGB(22,243,248);
 cv_begin(240,240-ACCESS_SPLASH_ROWS,navy);
 cv_rect(28,8,184,4,RGB(6,49,69));
 cv_rect(28,8,184*percent/100,4,cyan);
 access_center(240,18,&FONT_T,"LOADING",ACCESS_LABEL);
 cv_blit(0,ACCESS_SPLASH_ROWS);
}
static int access_drawn_page=-1;
void app_splash(void){
 access_drawn_page=-1;redraw=1;
 const uint8_t *packed=access_splash_pixels;
 unsigned block_end=0;
 int block_ok=0;
 uint8_t decoded[ACCESS_ART_BLOCK_ROWS*(ACCESS_ART_WIDTH/2)];
 for(unsigned y=0;y<ACCESS_SPLASH_ROWS;y+=24){
  unsigned rows=ACCESS_SPLASH_ROWS-y;
  if(rows>24)rows=24;
  cv_begin(240,rows,ACCESS_BG);
  for(unsigned row=0;row<rows;row++){
   unsigned source_row=(y+row)*ACCESS_ART_HEIGHT/ACCESS_SPLASH_ROWS;
   if(source_row>=block_end){
    unsigned block_rows=ACCESS_ART_HEIGHT-block_end;
    if(block_rows>ACCESS_ART_BLOCK_ROWS)block_rows=ACCESS_ART_BLOCK_ROWS;
    unsigned bytes=packed[0]|(unsigned)packed[1]<<8;
    packed+=2;
    block_ok=vc_lz4_unpack(packed,bytes,decoded,block_rows*(ACCESS_ART_WIDTH/2));
    packed+=bytes;
    block_end+=block_rows;
   }
   if(!block_ok)continue;
   const uint8_t *scanline=decoded+(source_row%ACCESS_ART_BLOCK_ROWS)*(ACCESS_ART_WIDTH/2);
   for(unsigned x=0;x<240;x++){
    unsigned sx=x*ACCESS_ART_WIDTH/240;
    unsigned pair=scanline[sx/2];
    cv_px[row*240+x]=swap16(access_splash_palette[(sx&1)?pair&15:pair>>4]);
   }
  }
  cv_blit(0,y);
 }
 access_splash_progress(3);
}
static unsigned access_wave_progress(void){
 unsigned bank[6];vc_band_device_stats(bank);
 return 70+25*(bank[0]>6?6:bank[0])/6;
}

static void access_knob_draw(unsigned step){
 const int cx=60,cy=30;
 for(unsigned i=1;i<=32;i++){
  uint16_t color=i<=step?ACCESS_AMBER:RGB(55,53,53);
  int x0=cx+access_arc_x[i-1],y0=cy+access_arc_y[i-1];
  int x1=cx+access_arc_x[i],y1=cy+access_arc_y[i];
  cv_line(x0,y0,x1,y1,color);
  cv_line(x0+1,y0,x1+1,y1,color);
  cv_line(x0,y0+1,x1,y1+1,color);
 }
 for(int y=0;y<49;y++)for(int x=0;x<49;x++){
  unsigned index=access_knob[y*49+x];
  if(index)cv_pset(cx-24+x,cy-24+y,access_knob_palette[index]);
 }
 int dx=access_pointer_x[step],dy=access_pointer_y[step];
 int side_x=dy<0?-1:1,side_y=dx<0?1:-1;
 cv_line(cx+dx/2,cy+dy/2,cx+dx,cy+dy,ACCESS_LABEL);
 cv_line(cx+dx/2+side_x,cy+dy/2+side_y,cx+dx+side_x,cy+dy+side_y,ACCESS_LABEL);
}

typedef struct {
 const char *label;
 char value[40];
 int step; /* -1 for a reading, 0..32 for an encoder. */
} AccessCell;

static void access_parameter(AccessCell *cell,int id){
 cell->label=labels[id];
 vc_value(id,cell->value);
 if((id==VC_WAVE1||id==VC_WAVE2)&&values[id]<10){
  cell->value[2]=0;cell->value[1]=cell->value[0];cell->value[0]='0';
 }
 int low=minparam(id),span=maxparam(id)-low;
 cell->step=span?(clamp(values[id]-low,0,span)*32+span/2)/span:0;
}

static void access_program_number(char out[4], unsigned program){
 out[0]=(char)('0'+program/100);
 out[1]=(char)('0'+program/10%10);
 out[2]=(char)('0'+program%10);
 out[3]=0;
}

static const char *access_sequence_name(unsigned slot,char *name){
 if(sequence_bad)return "INVALID BANK";
 if(!sequence_occupied(slot))return "EMPTY SLOT";
 sequence_name_for(slot,name);
 return name[0]?name:"UNTITLED";
}

static void access_draw_browser(int changed){
 static unsigned last_choice,last_loaded,last_wave,last_volume;
 int sequences=page==ACCESS_USER_SEQUENCES,user=page!=PAGE_PRESETS;
 unsigned selected=sequences?sequence_slot:user?user_slot:preset_choice,loaded=preset_loaded,volume=values[VC_PATCH_VOLUME];
 unsigned editing=sequences?sequence_name_edit:user&&user_name_edit;
 unsigned name_pos=sequences?sequence_name_pos:user_name_pos;
 const char *draft=sequences?sequence_name:user_name;
 if(!user&&!changed&&selected==last_choice&&loaded==last_loaded&&wave_state==last_wave&&volume==last_volume)return;
 unsigned bank=vc_preset_bank(selected),count=vc_preset_count();
 unsigned first=user?0:vc_preset_bank_first(bank);
 unsigned end=sequences?USER_SEQ_SLOTS:user?VC_PATCH_SLOTS:bank+1<vc_preset_bank_count()?vc_preset_bank_first(bank+1):count;
 unsigned top=selected<end&&selected>=first+2?selected-2:first;
 if(top+4>end)top=end>=first+4?end-4:first;
 char bank_text[16]="BANK ",program[4],caption[20],level[14]="K4  VOL ",name[VC_PATCH_NAME+1];
 vc_copy(bank_text+5,user?"USER":vc_preset_bank_label(bank));
 access_program_number(program,user?selected+1:vc_preset_program(selected));
 vc_copy(caption,user?"SLOT ":"PROGRAM ");
 vc_copy(caption+(user?5:8),selected<end?program:"---");
 const char *title=sequences?"USER SEQUENCES":user?"USER PATCHES":"FACTORY PRESETS";
 const char *tab=sequences?"SCL 3":user?"SCL 2":"SCL 1";
 const char *selected_name=user?(editing?draft:sequences?access_sequence_name(selected,name):vc_user_name(selected,name)):
  selected<count?vc_preset_name(selected):user_loaded?user_active.name:"INIT PATCH";
 cv_begin(240,32,ACCESS_RED);
 cv_text(8,2,&FONT_M,title,ACCESS_LABEL);
 cv_text(232-text_w(&FONT_XS,tab),9,&FONT_XS,tab,ACCESS_LABEL);
 cv_rect(0,31,240,1,ACCESS_RULE);
 cv_blit(0,0);
 cv_begin(240,43,ACCESS_BG);
 cv_text(8,5,&FONT_XS,bank_text,ACCESS_AMBER);
 cv_text(232-text_w(&FONT_XS,caption),5,&FONT_XS,caption,ACCESS_SUB);
 cv_text(8,19,&FONT_M,selected_name,ACCESS_LABEL);
 if(editing){
  int x=8;
  char glyph[2]={0};
  for(unsigned i=0;i<name_pos;i++){glyph[0]=draft[i];x+=text_w(&FONT_M,glyph);}
  glyph[0]=draft[name_pos];
  cv_rect(x,40,text_w(&FONT_M,glyph),1,ACCESS_AMBER);
 }
 cv_blit(0,32);
 for(unsigned row=0;row<4;row++){
  unsigned top_pad=row==0?3u:0u;
  cv_begin(240,30+top_pad,ACCESS_BG);
  unsigned item=top+row;
  if(item<end){
   unsigned active=item==selected;
   cv_rect(6,3+top_pad,228,27,active?RGB(110,63,37):ACCESS_RULE);
   cv_rect(7,4+top_pad,226,25,active?RGB(51,24,25):RGB(24,23,23));
   if(active)cv_rect(6,3+top_pad,3,27,ACCESS_AMBER);
   access_program_number(program,user?item+1:vc_preset_program(item));
   cv_text(16,8+top_pad,&FONT_XS,program,active?ACCESS_AMBER:ACCESS_SUB);
   cv_text(47,7+top_pad,&FONT_B,sequences?access_sequence_name(item,name):user?vc_user_name(item,name):vc_preset_name(item),ACCESS_LABEL);
   if(sequences?item==sequence_active:user&&user_loaded&&!wave_state&&item==user_active_slot)
    cv_text(225-text_w(&FONT_XS,"LIVE"),8+top_pad,&FONT_XS,"LIVE",ACCESS_AMBER);
  }
  cv_blit(0,78+30*row-top_pad);
 }
 cv_begin(240,42,ACCESS_BG);
 cv_rect(0,6,240,1,ACCESS_RULE);
 cv_rect(119,11,1,26,ACCESS_RULE);
 cv_text(8,9,&FONT_XS,user?(editing?"K1  DEST":"K1  SLOT"):"K1  BANK",ACCESS_LABEL);
 cv_text(128,9,&FONT_XS,user?"K2  RECALL":"K2  PRESET",ACCESS_LABEL);
 if(sequences){
  cv_text(8,24,&FONT_XS,sequence_job?"WORKING":sequence_status?user_status_names[sequence_status]:"SAVE  STORE",ACCESS_AMBER);
  cv_text(128,24,&FONT_XS,"K3 / K4  NAME",ACCESS_SUB);
 }else if(user){
  const char *state=user_pending_slot<VC_PATCH_SLOTS?"QUEUED":user_job?"WORKING":
   wave_state==3?"WAVE ERROR":wave_state?"LOADING":user_status?user_status_names[user_status]:"SAVE  STORE";
  cv_text(8,24,&FONT_XS,state,ACCESS_AMBER);
  cv_text(128,24,&FONT_XS,"K3 / K4  NAME",ACCESS_SUB);
 }else{
  const char *state=wave_state==3?"WAVE ERROR":wave_state||selected!=loaded?"LOADING":"K3  INIT";
  cv_text(8,24,&FONT_XS,state,ACCESS_SUB);
  vc_num(level+8,volume);
  cv_text(128,24,&FONT_XS,level,ACCESS_AMBER);
  last_choice=selected;last_loaded=loaded;last_wave=wave_state;last_volume=volume;
 }
 cv_blit(0,198);
}

static void access_status_cell(unsigned i,AccessCell *cell,const perf_stats_t *ps,const unsigned bank[6]){
 unsigned value=0;
 if(page==PAGE_PERF){
  const char *const names[]={"CPU AVG %","CPU PEAK %","XRUNS","DEADLINES"};
  const unsigned data[]={ps->audio_pct,ps->peak_pct,ps->dropouts,ps->deadline_misses};
  cell->label=names[i];value=data[i];
 }else if(page==ACCESS_CORE_TIME){
  const char *const names[]={"CPU1 AVG US","CPU0 WAIT US","CPU1 PEAK US","WAIT PEAK US"};
  const unsigned data[]={ps->worker_us,ps->wait_us,ps->worker_peak_us,ps->wait_peak_us};
  cell->label=names[i];value=data[i];
 }else if(page==ACCESS_CORE_STATE){
  const char *const names[]={"DUAL ACTIVE","CORE STATUS","JOBS","FAULTS"};
  const unsigned data[]={ps->dual,ps->status,ps->jobs,ps->faults};
  cell->label=names[i];value=data[i];
 }else if(page==ACCESS_AUDIO_STATS){
  const char *const names[]={"VOICES","QUEUE LOST","CLIPPED","BLOCK FRAMES"};
  const unsigned data[]={active_voices,queue_lost,clipped_samples,APP_BLOCK};
  cell->label=names[i];value=data[i];
 }else if(page==PAGE_PROFILE){
  const char *const names[]={"READY /32","WAIT /24","CACHE FILLS","FLASH HITS"};
  cell->label=names[i];value=i==3?vc_wave_flash_hits():bank[i];
 }else{
  const char *const names[]={"BUILD BUDGET","FAILURES","BUILD %","WORK MAX MS"};
  const unsigned data[]={band_budget,bank[4],bank[5],band_work_peak_ms};
  cell->label=names[i];value=data[i];
  if(i==0)cell->step=(band_budget-256)*32/(8192-256);
 }
 vc_num(cell->value,value);
}

/* Keep LCD transfers bounded when MIDI changes a value or a counter ticks. */
static char access_drawn_name[32];
static AccessCell access_drawn_cell[4];

static void access_draw_cell(unsigned i,const AccessCell *cell,int force){
 AccessCell *last=&access_drawn_cell[i];
 if(!force&&last->step==cell->step&&last->label==cell->label&&!memcmp(last->value,cell->value,sizeof(last->value)))return;
 *last=*cell;
 cv_begin(120,86,ACCESS_BG);
 if(i&1)cv_rect(0,0,1,86,ACCESS_RULE);
 if(i>=2)cv_rect(0,0,120,1,ACCESS_RULE);
 if(cell->step>=0){
  char label[44]={'K',(char)('1'+i),' ',' '};
  vc_copy(label+4,cell->label);
  access_knob_draw((unsigned)cell->step);
  access_center(120,55,&FONT_XS,label,ACCESS_LABEL);
  access_center(120,69,&FONT_B,cell->value,ACCESS_AMBER);
 }else{
  access_center(120,22,&FONT_XS,cell->label,ACCESS_LABEL);
  access_center(120,42,&FONT_M,cell->value,ACCESS_AMBER);
 }
 cv_blit(120*(i&1),34+86*(i>>1));
}

static void access_draw_footer(void){
 char status[40];
 const char *nav="ALGO  NEXT PAGE";
 const char *info="";
 if(page==ACCESS_CACHE_TUNE){nav="ALGO  NEXT PAGE   K2 RESET";info="K1  ADJUST BUILD BUDGET";}
 else if(access_profile_page()){nav="ALGO  NEXT PAGE   K2 RESET";info="GLO  NEXT STATUS PAGE";}
 if(wave_state==3){
  const char *stage=wave_error_stage?wave_error_stage:"UNKNOWN";
  unsigned end=0;while(stage[end]){status[end]=stage[end];end++;}status[end++]=' ';
  vc_num(status+end,wave_error_index);info=status;nav="WAVE DATA ERROR";
 }else if(wave_state)info="LOADING WAVES - MUTED";
 else if(wave_embedded&&page==ACCESS_CORE_STATE){
  const char digits[]="0123456789ABCDEF";
  uint32_t id=plat_flash_jedec();
  vc_copy(status,"EMBEDDED WAVES / FLASH ");
  for(unsigned i=0;i<6;i++)status[23+i]=digits[(id>>(20-4*i))&15u];
  status[29]=0;info=status;
 }
 cv_begin(240,34,ACCESS_BG);
 cv_rect(0,0,240,1,ACCESS_RULE);
 access_center(240,*info?2:8,&FONT_XS,nav,ACCESS_LABEL);
 if(*info)access_center(240,18,&FONT_T,info,ACCESS_SUB);
 cv_blit(0,206);
}

#include "generative_draw.inc"

static void vc_draw(void){
 if(access_boot_loading){
  if(wave_state==1||wave_state==2){access_splash_progress(access_wave_progress());return;}
  access_boot_loading=0;access_drawn_page=-1;
 }
 const AccessPage *panel=&access_pages[access_page_index()];
 int changed=access_drawn_page!=page;
 if(page==PAGE_PRESETS||page==PAGE_USER||page==ACCESS_USER_SEQUENCES){access_draw_browser(changed);access_drawn_page=page;return;}
 if(access_seq_page()){
  access_draw_generative(changed);access_drawn_page=page;return;
 }
 char name[32]={0};
 vc_copy(name,user_loaded?user_active.name:preset_loaded<vc_preset_count()?vc_preset_name(preset_loaded):"INIT PATCH");
 if(changed||memcmp(access_drawn_name,name,sizeof(name))){
  cv_begin(240,34,ACCESS_RED);
  access_center(240,-2,&FONT_M,panel->title,ACCESS_LABEL);
  access_center(240,18,&FONT_T,name,ACCESS_LABEL);
  cv_blit(0,0);
  memcpy(access_drawn_name,name,sizeof(name));
 }
 perf_stats_t ps;
 unsigned bank[6];
 if(access_profile_page()){perf_read(&ps);vc_band_device_stats(bank);}
 for(unsigned i=0;i<4;i++){
  AccessCell cell={"",{0},-1};
  if(access_profile_page())access_status_cell(i,&cell,&ps,bank);
  else if(panel->control[i]>=0){
   access_parameter(&cell,panel->control[i]);
   if((page==PAGE_FILTER||page==ACCESS_FILTER_2)){
    static const char *const names[]={"CUTOFF","RESONANCE","ENV AMOUNT","KEY TRACK"};
    cell.label=names[i];
   }
  }
  access_draw_cell(i,&cell,changed);
 }
 access_draw_footer();
 access_drawn_page=page;
}

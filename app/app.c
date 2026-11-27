#include "virooos.h"
#include "perf.h"
#include "wave_cache.h"
#include "presets.h"
#include "../runtime/wavetable_flash.h"
#ifdef VC_BAND_CACHE
void vc_band_device_attach(VCSynth *);
void vc_band_device_service(unsigned);
void vc_band_device_stats(unsigned[6]);
static unsigned band_budget=1024,band_work_peak_ms;
#define VC_PROFILE_KEYS 1
#include "../runtime/patch_record.h"
#include "../runtime/patch_sysex.h"
int vc_band_device_prepare(const unsigned[3],unsigned);
#else
#define VC_PROFILE_KEYS (page!=PAGE_PROFILE)
#endif
#ifdef PLAT_HOST
#include <string.h>
#endif
/* Audio owns the synth. UI is the sole producer of this bounded queue. */
typedef struct {uint8_t type,a,b;} VCEvent;
static VCEvent events[128];
static volatile uint32_t event_write,event_read,queue_lost,panic_requested;
#define VC_BARRIER() __asm__ volatile("" ::: "memory")
static VCSynth synth;
static volatile unsigned preset_loaded=~0u;
#define NO_PRESET_REQUEST (~0u)
/* The audio callback owns requests and the active matrix. The main loop
 * unpacks into staging and publishes it by storing the prepared index. */
static volatile unsigned preset_requested=NO_PRESET_REQUEST, preset_prepared=NO_PRESET_REQUEST;
static VCFactoryPreset prepared_factory;
static VCMatrix active_factory_matrix;
#ifdef VC_BAND_CACHE
static uint8_t user_bank[VC_PATCH_BANK_BYTES],user_candidate[VC_PATCH_BANK_BYTES];
static VCPatchRecord user_snapshot,user_recall,user_active;
static unsigned user_slot,user_bank_bad,user_name_pos;
static char user_name[11];
static volatile unsigned user_name_edit;
static unsigned user_remote_pending;
static void vc_user_reply(unsigned,unsigned);
static volatile unsigned user_job,user_status,user_loaded;
/* job: 0 idle,1 save queued,2 snapshot published,3 recall queued.
 * status:0 ready,1 saved,2 recalled,3 empty,4 busy,5 failed,6 bad bank. */
static const char *const user_status_names[]={"READY","SAVED","RECALLED","EMPTY","BUSY","FAILED","BAD BANK"};
#endif

static unsigned preset_choice=~0u,midi_preset_bank;
/* 1=request mute, 2=publish caches, 3=error. Audio alone acknowledges mute. */
static volatile unsigned wave_state,wave_ack,wave_desired[3],wave_ready[3],osc3_desired_mode;
static volatile unsigned lfo_desired[3],lfo_ready[3];
static int lfo_build_slot=-1,lfo_build_shape=-1;
static const unsigned lfo_shape_param[3]={VC_LFO_SHAPE,VC_LFO2_SHAPE,VC_LFO3_SHAPE};
static unsigned ui_wave_state;
static int wave_build_slot=-1,wave_build_id=-1;
static unsigned wave_embedded;
static const char *wave_error_stage;
static unsigned wave_error_index;
static int vc_wave_fail(const char *stage,unsigned index){
 wave_error_stage=stage;wave_error_index=index;wave_state=3;return -1;
}
/* LFO extension occupies only the unused WAV area, below settings at DC000.
 * Reconstruct and verify while muted; no new RAM buffers or audio-time decoding. */
static int vc_lfo_needs_cache(unsigned shape){return shape>=6;}
static void vc_wave_background(void){
 if(wave_state!=1||!wave_ack)return;
 /* Prepare LFO copies before restoring the oscillator caches they borrow. */
 for(unsigned i=0;i<3;i++){
  unsigned shape=lfo_desired[i];
  if(vc_lfo_active_shape(i)==shape)continue;
  wave_build_slot=wave_build_id=-1;
  if(!vc_lfo_needs_cache(shape)){vc_lfo_active_set(i,shape,0);lfo_build_slot=lfo_build_shape=-1;return;}
  if(lfo_build_slot!=(int)i||lfo_build_shape!=(int)shape){
   if(!vc_wave_begin(2,shape-4)){vc_wave_fail("ACTIVE LFO START",shape);return;}
   lfo_build_slot=(int)i;lfo_build_shape=(int)shape;
  }
  int result=vc_wave_service(32768);
  if(result<0){vc_wave_fail("ACTIVE LFO REBUILD",shape);return;}
  if(result){
   if(!vc_lfo_active_set(i,shape,vc_wave_table(shape-4,0))){vc_wave_fail("ACTIVE LFO COPY",shape);return;}
   lfo_build_slot=lfo_build_shape=-1;
  }
  return;
 }
 lfo_build_slot=lfo_build_shape=-1;
 unsigned desired[3]={wave_desired[0],wave_desired[1],wave_desired[2]};
#ifdef VC_BAND_CACHE
 int preparation=vc_band_device_prepare(desired,32768);
 if(preparation<0){vc_wave_fail("OSC CACHE PRELOAD",0);return;}
 if(!preparation)return;
#endif
 int slot=-1;
#ifndef VC_BAND_CACHE
 if(vc_wave_cached(0)!=(int)desired[0])slot=0;
 else if(desired[1]!=desired[0]&&vc_wave_cached(1)!=(int)desired[1])slot=1;
 else if(desired[2]!=desired[0]&&desired[2]!=desired[1]&&vc_wave_cached(2)!=(int)desired[2])slot=2;
#endif
 if(slot<0){for(unsigned i=0;i<3;i++)lfo_ready[i]=lfo_desired[i];wave_build_slot=-1;wave_ready[0]=desired[0];wave_ready[1]=desired[1];wave_ready[2]=desired[2];VC_BARRIER();wave_state=2;return;}
 if(slot!=wave_build_slot||(int)desired[slot]!=wave_build_id){
  if(!vc_wave_begin((unsigned)slot,desired[slot])){vc_wave_fail("OSC REBUILD START",desired[slot]);return;}
  wave_build_slot=slot;wave_build_id=(int)desired[slot];
 }
 if(vc_wave_service(4096)<0){vc_wave_fail("OSC REBUILD",wave_build_id);return;}
}
/* XIP-to-SRAM exceeds the target direct-call range. One indirect dispatch per
 * audio block; calls within the SRAM DSP remain direct. */
#ifndef PLAT_HOST
static void (*volatile vc_render_dispatch)(VCSynth*,float*,float*,unsigned)=vc_render_stereo;
#else
#define vc_render_dispatch vc_render_stereo
#endif
static float audio_left[APP_BLOCK],audio_right[APP_BLOCK],master_now;
static int redraw;
static uint8_t values[VC_PARAMS];
/* Audio publishes one aligned value+generation word per MIDI-controlled
 * parameter. UI never clears it, so a concurrent new update cannot be lost. */
static volatile uint32_t midi_values[VC_PARAMS];
static uint32_t midi_seen[VC_PARAMS];
static void vc_midi_publish(unsigned id,unsigned value){
 uint32_t next=((midi_values[id]+256u)&~255u)|(value&255u);
 VC_BARRIER();midi_values[id]=next;
}
static void vc_midi_ui(void){
 for(unsigned id=0;id<VC_PARAMS;id++){
  uint32_t value=midi_values[id];if(value==midi_seen[id])continue;
  midi_seen[id]=value;values[id]=(uint8_t)value;redraw=1;
 }
}
static int page,row,octave,key_note[NKEYS];
#ifdef VC_ACCESS_UI
static unsigned access_boot_loading;
#endif
static uint32_t last_keys,last_buttons,last_draw;
static volatile unsigned active_voices,clipped_samples;
enum {PAGE_OSC,PAGE_FILTER,PAGE_ENV,PAGE_FMOD,PAGE_STEREO,PAGE_PERF,PAGE_PROFILE,PAGE_NOISE,PAGE_FM,PAGE_LFO1,PAGE_LFO_FILTER,PAGE_LFO_TIMING,PAGE_LFO2,PAGE_LFO2_OSC,PAGE_LFO3,PAGE_OSC3,PAGE_SAT,PAGE_VELOCITY,PAGE_VEL_OSC,PAGE_AMP_RESPONSE,PAGE_PITCH,PAGE_LFO_SHARED,PAGE_DIST,PAGE_EQ,PAGE_PRESETS,PAGE_UNISON,PAGE_BOOST
#ifdef VC_BAND_CACHE
,PAGE_USER
#endif
};
#ifdef VC_BAND_CACHE
#define PAGE_LAST PAGE_USER
#else
#define PAGE_LAST PAGE_BOOST
#endif
#ifdef VC_ACCESS_UI
#include "access_pages.inc"
#else
static const int8_t pages[24][8]={
 {VC_SHAPE1,VC_SHAPE2,VC_BALANCE,VC_DETUNE,VC_WAVE1,VC_WAVE2,VC_SYNC,VC_CUTOFF2},
 {VC_CUTOFF,VC_RESONANCE,VC_FILTER_MODE,VC_ROUTING,VC_CUTOFF2,VC_RESONANCE2,VC_FILTER_MODE2,VC_FILTER_BALANCE},
 {VC_ATTACK,VC_DECAY,VC_SUSTAIN,VC_RELEASE,VC_F_ATTACK,VC_F_DECAY,VC_F_SUSTAIN,VC_F_RELEASE},
 {VC_F_AMOUNT1,VC_F_AMOUNT2,VC_F_POLARITY1,VC_F_POLARITY2,VC_CUTOFF_LINK,VC_F_KEYFOLLOW1,VC_F_KEYFOLLOW2,VC_F_KEYBASE},
 {VC_AMP_SLOPE,VC_F_SLOPE,VC_PAN,VC_SPREAD,VC_SUB_VOLUME,VC_SUB_SHAPE,VC_RING_VOLUME,VC_INIT_PHASE},
 {VC_NOISE_VOLUME,VC_NOISE_COLOR,VC_PW1,VC_PW2,VC_TUNE1,VC_TUNE2,VC_KEYFOLLOW1,VC_KEYFOLLOW2},
 {VC_FM_AMOUNT,VC_FM_MODE,VC_FM_ENV,VC_FM_VELOCITY,VC_OSC2_ENV,VC_F_ATTACK,VC_F_DECAY,VC_F_SUSTAIN},
 {VC_LFO_RATE,VC_LFO_SHAPE,VC_LFO_CONTOUR,VC_LFO_TRIGGER,VC_LFO_OSC1,VC_LFO_OSC2,VC_LFO_PW,VC_LFO_MODE},
 {VC_LFO_RESONANCE,VC_LFO_FILTER_GAIN,VC_LFO_RATE,VC_LFO_MODE,VC_RESONANCE,VC_RESONANCE2,VC_CUTOFF,VC_CUTOFF2},
 {VC_LFO_CLOCK,VC_TEMPO,VC_LFO_KEYTRACK,VC_LFO_MODE,VC_LFO_RATE,VC_LFO_TRIGGER,VC_LFO_SHAPE,VC_LFO_CONTOUR},
 {VC_LFO2_RATE,VC_LFO2_SHAPE,VC_LFO2_CONTOUR,VC_LFO2_TRIGGER,VC_LFO2_CUTOFF1,VC_LFO2_CUTOFF2,VC_LFO2_MODE,VC_LFO2_KEYTRACK},
 {VC_LFO2_SHAPE_AMOUNT,VC_LFO2_FM_AMOUNT,VC_LFO2_CLOCK,VC_LFO2_MODE,VC_LFO2_RATE,VC_TEMPO,VC_LFO2_SHAPE,VC_LFO2_PAN},
 {VC_LFO3_RATE,VC_LFO3_SHAPE,VC_LFO3_AMOUNT,VC_LFO3_DEST,VC_LFO3_FADE,VC_LFO3_KEYTRACK,VC_LFO3_CLOCK,VC_LFO3_ENABLE},
 {VC_OSC3_MODE,VC_OSC3_VOLUME,VC_OSC3_TUNE,VC_OSC3_DETUNE,VC_SHAPE2,VC_PW2,VC_TUNE2,VC_DETUNE},
 {VC_SAT_CURVE,VC_SAT_DRIVE,VC_ROUTING,VC_FILTER_BALANCE,VC_CUTOFF,VC_RESONANCE,VC_CUTOFF2,VC_PUNCH},
 {VC_VEL_ENV1,VC_VEL_ENV2,VC_VEL_RES1,VC_VEL_RES2,VC_F_AMOUNT1,VC_F_AMOUNT2,VC_RESONANCE,VC_RESONANCE2},
 {VC_VEL_SHAPE1,VC_VEL_SHAPE2,VC_VEL_PW,VC_VEL_PAN,VC_SHAPE1,VC_SHAPE2,VC_PW1,VC_PW2},
 {VC_AMP_VELOCITY,VC_PATCH_VOLUME,VC_DECAY,VC_SUSTAIN,VC_RELEASE,VC_AMP_SLOPE,VC_PAN,VC_SAT_DRIVE},
 {VC_TRANSPOSE,VC_BEND_UP,VC_BEND_DOWN,VC_BEND_SCALE,VC_TUNE1,VC_TUNE2,VC_KEY_MODE,VC_PORTAMENTO},
 {VC_LFO_GLOBAL,VC_LFO2_GLOBAL,VC_LFO_MODE,VC_LFO2_MODE,VC_LFO_TRIGGER,VC_LFO2_TRIGGER,VC_LFO_RATE,VC_LFO3_GLOBAL},
 {VC_DIST_MODE,VC_DIST_LEVEL,-1,-1,-1,-1,-1,-1},
 {VC_EQ_ENABLE,VC_EQ_LOW_GAIN,VC_EQ_LOW_FREQ,VC_EQ_MID_GAIN,VC_EQ_MID_FREQ,VC_EQ_MID_Q,VC_EQ_HIGH_GAIN,VC_EQ_HIGH_FREQ},
 {VC_UNISON_MODE,VC_UNISON_DETUNE,VC_SPREAD,VC_UNISON_PHASE,VC_KEY_MODE,VC_PORTAMENTO,-1,-1},
 {VC_BOOST_LEVEL,VC_BOOST_TUNE,-1,-1,-1,-1,-1,-1}
};
static const char *const page_names[]={"OSCILLATORS","FILTERS / ROUTING","AMP / FILTER ENVELOPES","FILTER MODULATION","STEREO / EXTRA SOURCES","PERFORMANCE","MUTED HARDWARE PROFILE","OSC EXTRAS / NOISE","OSCILLATOR FM","LFO 1 / OSCILLATORS","LFO 1 / FILTERS","LFO 1 / TIMING","LFO 2 / FILTER CUTOFF","LFO 2 / OSCILLATORS","LFO 3 / PITCH AND PWM","OSCILLATOR 3","FILTER SATURATION","FILTER VELOCITY","OSC / PAN VELOCITY","AMPLIFIER RESPONSE","PITCH / BEND","SHARED LFO MODES","POST-MIX DISTORTION","POST-MIX EQ","FACTORY PRESETS","UNISON","ANALOG BOOST"
#ifdef VC_BAND_CACHE
,"USER PATCHES"
#endif
};
static const char *const profile_names[]={"FULL","OSC / SUB / RING","FILTERS","ENVELOPES","CONTROL","IDLE"};
#endif
static const char *const labels[VC_PARAMS]={
 "SHAPE 1","SHAPE 2","OSC BAL","DETUNE","CUTOFF 1","RES 1","MODE 1","ROUTING",
 "AMP ATK","AMP DEC","AMP SUS","AMP REL","WAVE 1","WAVE 2","OSC SYNC","CUTOFF 2",
 "FILT BAL","MODE 2","RES 2","FILT ATK","FILT DEC","FILT SUS","FILT REL",
 "ENV AMT 1","ENV AMT 2","POLARITY 1","POLARITY 2","CUT LINK","KEYFLW 1","KEYFLW 2","KEY BASE",
 "AMP SLOPE","FILT SLOPE","PAN","STEREO WIDTH","SUB LEVEL","SUB SHAPE","RING LEVEL","NOISE LEVEL","NOISE COLOR","WIDTH 1","WIDTH 2","TUNE 1 ST","TUNE 2 ST","KEYTRK 1 %","KEYTRK 2 %","FM AMOUNT","FM SOURCE","FM FILT ENV","FM VELOCITY","OSC2 ENV PITCH","OSC START","LFO RATE","LFO SHAPE","CONTOUR","TRIGGER PHASE","OSC1 PITCH","OSC2 PITCH","PULSE WIDTH","LFO MODE","LFO RESONANCE","LFO FILT GAIN","CLOCK DIVISION","TEMPO BPM","LFO KEYTRACK","LFO2 RATE","LFO2 SHAPE","LFO2 CONTOUR","LFO2 TRIGGER","LFO2 CUTOFF 1","LFO2 CUTOFF 2","LFO2 MODE","LFO2 KEYTRACK","LFO2 SHAPE AMT","LFO2 FM AMT","LFO2 CLOCK","LFO2 PAN AMT","LFO3 RATE","LFO3 SHAPE","LFO3 AMOUNT","DESTINATION","LFO3 FADE IN","LFO3 KEYTRACK","LFO3 CLOCK","LFO3 ENABLE","OSC3 MODE","OSC3 VOLUME","OSC3 TUNE ST","OSC3 DETUNE","SAT CURVE","OSC VOLUME","PUNCH","VEL ENV 1","VEL ENV 2","VEL RES 1","VEL RES 2","VEL SHAPE 1","VEL SHAPE 2","VEL WIDTH","VEL PAN","AMP VELOCITY","TRANSPOSE","BEND UP","BEND DOWN","BEND CURVE","PORTAMENTO","PATCH VOLUME","KEY MODE","LFO1 SCOPE","LFO2 SCOPE","DIST MODE","DIST INTENSITY","EQ ENABLE","LOW GAIN","LOW FREQ","MID GAIN","MID FREQ","MID Q","HIGH GAIN","HIGH FREQ","LFO3 SCOPE","UNISON VOICES","UNISON DETUNE","UNISON LFO PHASE","BOOST INTENSITY","BOOST TUNE"
};
static int minparam(int id){return id==VC_TUNE1||id==VC_TUNE2||id==VC_OSC3_TUNE?16:0;}
static int maxparam(int id){return id==VC_TUNE1||id==VC_TUNE2||id==VC_OSC3_TUNE?112:(int)vc_param_max((unsigned)id);}
static void vc_post(unsigned type,unsigned a,unsigned b){uint32_t w=event_write;if(w-event_read>=128){queue_lost++;panic_requested=1;return;}events[w&127]=(VCEvent){type,a,b};VC_BARRIER();event_write=w+1;}
/* Only the audio owner requests cache changes. MIDI and knobs use the same
 * mute/acknowledge/load/publish transaction. Repeating a selection is a no-op. */
static void vc_request_wave(unsigned which,unsigned wave){
 if(wave>=VC_WAVES)wave=VC_WAVES-1;
 if(wave_desired[which]==wave)return;
 wave_desired[which]=wave;
 if(which&&osc3_desired_mode<4)wave_desired[2]=wave;
#ifdef VC_BAND_CACHE
 if(!wave_state){vc_param(&synth,which?VC_WAVE2:VC_WAVE1,wave);return;}
#endif
 wave_state=1;wave_ack=0;
}
static int vc_request_lfo(unsigned id,unsigned shape){
 for(unsigned i=0;i<3;i++)if(id==lfo_shape_param[i]){
  if(shape>VC_LFO_MAX_SHAPE)shape=VC_LFO_MAX_SHAPE;
  if(lfo_desired[i]==shape)return 1;
  lfo_desired[i]=shape;
  if(!vc_lfo_needs_cache(shape)){
   /* These shapes already exist independently of the active RAM cache. */
   if(!wave_state){vc_param(&synth,id,shape);return 1;}
  }
  wave_state=1;wave_ack=0;return 1;
 }
 return 0;
}
static void vc_request_preset(unsigned index){
 if(index<=vc_preset_count())preset_requested=index;
}
static void vc_preset_background(void){
 unsigned index=preset_requested;
 if(index==NO_PRESET_REQUEST||index==preset_prepared)return;
 preset_prepared=NO_PRESET_REQUEST;
 VC_BARRIER();
 if(vc_preset_decode(index,&prepared_factory)&&index==preset_requested){
  VC_BARRIER();preset_prepared=index;
 }
}
static void vc_apply_factory(unsigned index){
#ifdef VC_BAND_CACHE
 user_loaded=0;user_name_edit=0;
#endif
 if(index<vc_preset_count()){
  active_factory_matrix=prepared_factory.matrix;
  vc_apply_patch(&synth,prepared_factory.params);
  vc_set_matrix(&synth,&active_factory_matrix);
 }else{
  vc_init(&synth,44100);
  vc_set_matrix(&synth,0);
 }
 preset_loaded=index;
 wave_desired[0]=synth.param[VC_WAVE1];wave_desired[1]=synth.param[VC_WAVE2];
 osc3_desired_mode=synth.param[VC_OSC3_MODE];wave_desired[2]=osc3_desired_mode>=4?osc3_desired_mode-4:wave_desired[1];
 for(unsigned i=0;i<3;i++)lfo_desired[i]=synth.param[lfo_shape_param[i]];
 wave_state=1;wave_ack=0;
}
#ifdef VC_BAND_CACHE
static void vc_user_audio(unsigned type){
 if(preset_requested!=NO_PRESET_REQUEST){user_status=4;VC_BARRIER();user_job=0;return;}
 if(type==9||type==11){
  if(wave_state){user_status=4;VC_BARRIER();user_job=0;return;}
  if(type==11)user_snapshot=user_recall;
  else {
  memset(&user_snapshot,0,sizeof(user_snapshot));
  const char *name=user_name_edit?user_name:user_loaded?user_active.name:vc_preset_name(preset_loaded);
  for(unsigned i=0;i<10&&name[i];i++)user_snapshot.name[i]=name[i];
  memcpy(user_snapshot.params,synth.param,VC_PARAMS);
  const VCMatrix *matrix=synth.matrix;
  if(matrix)user_snapshot.matrix=*matrix;
  else for(unsigned i=0;i<11;i++)user_snapshot.matrix.route[i].amount=64;
  }
  wave_state=1;wave_ack=0;VC_BARRIER();user_job=2;
 }else{
  if(wave_state){user_status=4;VC_BARRIER();user_job=0;return;}
  user_active=user_recall;vc_apply_patch(&synth,user_active.params);vc_set_matrix(&synth,&user_active.matrix);
  preset_loaded=vc_preset_count();user_loaded=1;user_name_edit=0;
  wave_desired[0]=synth.param[VC_WAVE1];wave_desired[1]=synth.param[VC_WAVE2];
  osc3_desired_mode=synth.param[VC_OSC3_MODE];wave_desired[2]=osc3_desired_mode>=4?osc3_desired_mode-4:wave_desired[1];
  for(unsigned i=0;i<3;i++)lfo_desired[i]=synth.param[lfo_shape_param[i]];
  wave_state=1;wave_ack=0;user_status=2;VC_BARRIER();user_job=0;
 }
}
static void vc_user_background(void){
 if(user_remote_pending&&!user_job){user_remote_pending=0;vc_user_reply(user_slot,1);}
 if(user_job!=2||!wave_ack)return;
 VC_BARRIER();memcpy(user_candidate,user_bank,sizeof(user_bank));
 if(!user_bank_bad&&vc_patch_bank_put(user_candidate,sizeof(user_candidate),user_slot,&user_snapshot)&&!plat_store_save(OBJ_SETTINGS,user_candidate,sizeof(user_candidate))){
  /* Verify flash readback before accepting the saved bank in RAM. */
  if(plat_store_load(OBJ_SETTINGS,user_bank,sizeof(user_bank))==(int)sizeof(user_bank)&&!memcmp(user_bank,user_candidate,sizeof(user_bank))&&vc_patch_bank_valid(user_bank,sizeof(user_bank)))user_status=1;
  else {user_status=5;user_bank_bad=1;}
 }else user_status=user_bank_bad?6:5;
 VC_BARRIER();user_job=0;redraw=1;
 if(user_remote_pending){user_remote_pending=0;vc_user_reply(user_slot,user_status==1?0:4);}
}
#endif
static void vc_audio_events(void){
 if(panic_requested){vc_panic(&synth);event_read=event_write;panic_requested=0;
#ifdef VC_BAND_CACHE
 if(user_job==1||user_job==3){user_status=5;VC_BARRIER();user_job=0;}
#endif
}
 while(event_read!=event_write){VC_BARRIER();VCEvent e=events[event_read&127];event_read++;
#ifdef VC_BAND_CACHE
 if(user_job==2)continue;
 if(e.type==9||e.type==10||e.type==11){vc_user_audio(e.type);continue;}
#endif
if(e.type==1){if(!wave_state)vc_note_on(&synth,e.a,e.b);}else if(e.type==2)vc_note_off(&synth,e.a);else if(e.type==3){if(e.a==VC_WAVE1||e.a==VC_WAVE2){vc_request_wave(e.a==VC_WAVE2,e.b);}
 else if(e.a==VC_OSC3_MODE){if(osc3_desired_mode==e.b)continue;osc3_desired_mode=e.b;unsigned w=e.b>=4?e.b-4:wave_desired[1];
#ifdef VC_BAND_CACHE
  if(!wave_state){wave_desired[2]=w;vc_param(&synth,e.a,e.b);continue;}
#endif
  if(wave_desired[2]!=w||e.b>=4){wave_desired[2]=w;wave_state=1;wave_ack=0;}else vc_param(&synth,e.a,e.b);
 }else if(!vc_request_lfo(e.a,e.b))vc_param(&synth,e.a,e.b);}else if(e.type==4)vc_panic(&synth);else if(e.type==5)clipped_samples=0;
 else if(e.type==8)vc_request_preset(e.a|((unsigned)e.b<<8));
}
 uint32_t pkt;int port;while(plat_midi_in(&pkt,&port)){
#ifdef VC_BAND_CACHE
 if(user_job==2)continue;
#endif
unsigned st=(pkt>>8)&240,a=(pkt>>16)&127,b=(pkt>>24)&127;
 if(st==176&&a==0){if(b<vc_preset_bank_count())midi_preset_bank=b;continue;}
 if(st==192){int i=vc_preset_find(midi_preset_bank,a);if(i>=0)vc_request_preset((unsigned)i);continue;}
 /* Wave requests are accepted even while muted, so the newest request wins.
  * Other MIDI retains the existing ignore-during-loading behavior. */
 if(st==176&&(a==19||a==24)){
  unsigned which=a==24;vc_request_wave(which,b);
  vc_midi_publish(which?VC_WAVE2:VC_WAVE1,wave_desired[which]);continue;
 }
 if(st==176){int id=vc_cc_param(a);if(id>=0&&vc_request_lfo((unsigned)id,b)){vc_midi_publish((unsigned)id,b>VC_LFO_MAX_SHAPE?VC_LFO_MAX_SHAPE:b);continue;}}
 if(wave_state)continue;
 if(st==144&&b)vc_note_on(&synth,a,b);else if(st==128||st==144)vc_note_off_velocity(&synth,a,st==128?b:0);else if(st==208)vc_pressure(&synth,a);else if(st==176){vc_cc(&synth,a,b);int id=vc_cc_param(a);if(id>=0)vc_midi_publish((unsigned)id,synth.param[id]);}else if(st==224)vc_bend(&synth,a+(b<<7));}
}
#ifdef VC_DUAL_RENDER
/* APP_OVERLOAD: the platform calls app_overload() after a block that used
 * over 90% of its time; the next block sheds one releasing voice. */
static volatile unsigned shed_request,shed_count;
void app_overload(void){shed_request=1;}
#endif
void app_render(int32_t *lr,uint32_t n,uint32_t master){
 vc_audio_events();
 unsigned index=preset_requested;
 if(index!=NO_PRESET_REQUEST&&index==preset_prepared){
  VC_BARRIER();vc_apply_factory(index);
  preset_prepared=preset_requested=NO_PRESET_REQUEST;
 }
#ifdef VC_DUAL_RENDER
 if(shed_request){shed_request=0;if(vc_shed(&synth))shed_count++;}
#endif
 if(wave_state==2)for(unsigned i=0;i<3;i++)if(lfo_ready[i]!=lfo_desired[i])wave_state=1;
 if(wave_state==2&&(wave_ready[0]!=wave_desired[0]||wave_ready[1]!=wave_desired[1]||wave_ready[2]!=wave_desired[2]))wave_state=1;
 if(wave_state==2){for(unsigned i=0;i<3;i++)vc_param(&synth,lfo_shape_param[i],lfo_desired[i]);vc_param(&synth,VC_WAVE1,wave_desired[0]);vc_param(&synth,VC_WAVE2,wave_desired[1]);vc_param(&synth,VC_OSC3_MODE,osc3_desired_mode);VC_BARRIER();wave_state=0;wave_ack=0;}
 if(wave_state){vc_panic(&synth);active_voices=0;memset(lr,0,n*2*sizeof(*lr));VC_BARRIER();wave_ack=1;return;}
 float target=master/4096.f;
 for(unsigned at=0;at<n;at+=APP_BLOCK){unsigned count=n-at;if(count>APP_BLOCK)count=APP_BLOCK;vc_render_dispatch(&synth,audio_left,audio_right,count);
 for(unsigned j=0;j<count;j++){
  master_now+=(target-master_now)*.004f;
  float l=audio_left[j]*master_now,r=audio_right[j]*master_now;
  int clipped=0;
  if(l>1){l=1;clipped=1;}if(l< -1){l=-1;clipped=1;}
  if(r>1){r=1;clipped=1;}if(r< -1){r=-1;clipped=1;}
  clipped_samples+=(unsigned)clipped;
  lr[2*(at+j)]=(int32_t)(l*8300000);lr[2*(at+j)+1]=(int32_t)(r*8300000);
 }}
 active_voices=vc_active(&synth);
}
void app_panic(void){vc_panic(&synth);event_read=event_write;active_voices=0;}
#ifdef PLAT_HOST
void perf_read(perf_stats_t *s){memset(s,0,sizeof(*s));}
void perf_reset(void){}
void perf_mode(int dual){(void)dual;}
void perf_scan_mode(unsigned div){(void)div;}
#endif
void app_init(void){preset_requested=preset_prepared=NO_PRESET_REQUEST;preset_loaded=preset_choice=vc_preset_count();midi_preset_bank=0;vc_init(&synth,44100);memcpy(values,synth.param,sizeof(values));memset((void*)midi_values,0,sizeof(midi_values));memset(midi_seen,0,sizeof(midi_seen));event_read=event_write=queue_lost=panic_requested=0;clipped_samples=0;last_keys=last_buttons=0;page=row=octave=0;master_now=0;for(int i=0;i<NKEYS;i++)key_note[i]=-1;perf_mode(
#ifdef VC_DUAL_RENDER
 1
#else
 0
#endif
 );redraw=1;
#ifdef VC_ACCESS_UI
 access_boot_loading=1;
#endif
 vc_lfo_active_reset();for(unsigned i=0;i<3;i++)lfo_desired[i]=lfo_ready[i]=synth.param[lfo_shape_param[i]];lfo_build_slot=lfo_build_shape=-1;
 vc_lfo_set_extra_tables(0);
 wave_state=1;ui_wave_state=255;wave_ack=0;wave_desired[0]=wave_desired[1]=wave_desired[2]=0;osc3_desired_mode=0;wave_build_slot=wave_build_id=-1;
 wave_embedded=vc_wave_init(vc_wave_seed(),vc_wave_seed_size());
 if(!wave_embedded)vc_wave_fail("EMBEDDED BANK FORMAT",0);
 const uint8_t *cache=plat_data(CONTAGION_CACHE_BASE);
 vc_wave_flash_init(cache && plat_data(CONTAGION_CACHE_END-1u)==cache+(CONTAGION_CACHE_END-CONTAGION_CACHE_BASE-1u)?cache:0);
#ifdef VC_BAND_CACHE
 vc_band_device_attach(&synth);
 user_slot=user_job=user_status=user_loaded=user_bank_bad=user_name_edit=user_name_pos=user_remote_pending=0;
 int bytes=plat_store_load(OBJ_SETTINGS,user_bank,sizeof(user_bank));
 if(bytes<0)vc_patch_bank_init(user_bank);
 else if(bytes!=(int)sizeof(user_bank)||!vc_patch_bank_valid(user_bank,sizeof(user_bank))){user_bank_bad=1;user_status=6;}
#endif
}
void app_say(const char *a,const char *b){(void)a;(void)b;redraw=1;}
#ifdef VC_BAND_CACHE
static void vc_user_edit_name(unsigned knob,int delta){
 if(!user_name_edit){
  const char *name=user_loaded?user_active.name:vc_preset_name(preset_loaded);
  memset(user_name,' ',10);user_name[10]=0;
  for(unsigned i=0;i<10&&name[i];i++)user_name[i]=name[i];
  user_name_pos=0;user_name_edit=1;
 }
 if(knob==EN_K3)user_name_pos=(unsigned)clamp((int)user_name_pos+delta,0,9);
 else user_name[user_name_pos]=(char)clamp((unsigned char)user_name[user_name_pos]+delta,32,126);
}
#endif
static void vc_input(void){
 vc_midi_ui();
#ifndef VC_BAND_CACHE
 int previous_page=page;
#endif
 touch_poll();uint32_t keys=touch_keys(),btn=plat_buttons(),pressed=btn&~last_buttons;
 for(int k=0;k<NKEYS;k++){uint32_t bit=1u<<k;if(VC_PROFILE_KEYS&&(keys&bit)&&!(last_keys&bit)){key_note[k]=53+k+12*octave;vc_post(1,key_note[k],touch_vel(k));}else if(!(keys&bit)&&(last_keys&bit)){if(key_note[k]>=0)vc_post(2,key_note[k],0);key_note[k]=-1;}}
#ifdef VC_BAND_CACHE
 if(pressed&(1u<<B_SAVE)){
  page=PAGE_USER;
  if(user_bank_bad)user_status=6;
  else if(user_job||wave_state)user_status=4;
  else {user_job=1;vc_post(9,0,0);}
  redraw=1;
 }
#endif
#ifdef VC_ACCESS_UI
 const uint8_t shortcuts[]={B_HOME,B_EDIT,B_ENV,B_LFO,B_FX,B_GLO,B_SEL};
 for(unsigned i=0;i<sizeof(shortcuts);i++)if(pressed&(1u<<shortcuts[i])){access_shortcut(shortcuts[i]);break;}
 if(pressed&(1u<<B_PLAY)){vc_post(4,0,0);redraw=1;}
#else
 if(pressed&(1u<<B_HOME)){page=0;row=0;redraw=1;}
 if(pressed&(1u<<B_ENV)){row=page==PAGE_ENV?!row:0;page=PAGE_ENV;redraw=1;}
 if(pressed&(1u<<B_EDIT)){page=PAGE_FILTER;row=0;redraw=1;}
 if(pressed&(1u<<B_LFO)){page=page==PAGE_FMOD?PAGE_LFO1:page==PAGE_LFO1?PAGE_LFO_FILTER:page==PAGE_LFO_FILTER?PAGE_LFO_TIMING:page==PAGE_LFO_TIMING?PAGE_LFO2:page==PAGE_LFO2?PAGE_LFO2_OSC:page==PAGE_LFO2_OSC?PAGE_LFO3:page==PAGE_LFO3?PAGE_LFO_SHARED:PAGE_FMOD;row=0;redraw=1;}
 if(pressed&(1u<<B_FX)){page=page==PAGE_STEREO?PAGE_NOISE:page==PAGE_NOISE?PAGE_FM:page==PAGE_FM?PAGE_OSC3:page==PAGE_OSC3?PAGE_SAT:page==PAGE_SAT?PAGE_VELOCITY:page==PAGE_VELOCITY?PAGE_VEL_OSC:page==PAGE_VEL_OSC?PAGE_AMP_RESPONSE:page==PAGE_AMP_RESPONSE?PAGE_PITCH:page==PAGE_PITCH?PAGE_DIST:page==PAGE_DIST?PAGE_EQ:page==PAGE_EQ?PAGE_PRESETS:page==PAGE_PRESETS?PAGE_UNISON:page==PAGE_UNISON?PAGE_BOOST:PAGE_STEREO;row=0;redraw=1;}
 if(pressed&(1u<<B_GLO)){page=page==PAGE_PERF?PAGE_PROFILE:PAGE_PERF;row=0;redraw=1;}
 if(pressed&(1u<<B_SEL)){row^=1;redraw=1;}
 if(pressed&(1u<<B_PLAY)){if(page==PAGE_PROFILE)page=PAGE_PERF;vc_post(4,0,0);redraw=1;}
#endif
 if(pressed&(1u<<B_OCTDN)){octave=clamp(octave-1,-2,2);redraw=1;}
 if(pressed&(1u<<B_OCTUP)){octave=clamp(octave+1,-2,2);redraw=1;}
 for(int k=0;k<NE;k++){int d=plat_enc(k);if(!d)continue;
 if(k==EN_PRESET){
 preset_choice=preset_choice>=vc_preset_count()?0:(unsigned)clamp((int)preset_choice+d,0,(int)vc_preset_count()-1);page=PAGE_PRESETS;row=0;vc_post(8,preset_choice&255,preset_choice>>8);redraw=1;
 }else if(page==PAGE_PRESETS&&k==EN_K3){preset_choice=vc_preset_count();vc_post(8,preset_choice&255,preset_choice>>8);redraw=1;
 }else if(page==PAGE_PRESETS&&(k==EN_K1||k==EN_K2)){
 if(k==EN_K1)preset_choice=vc_preset_bank_first((unsigned)clamp((int)vc_preset_bank(preset_choice)+d,0,(int)vc_preset_bank_count()-1));
 else {
  unsigned bank=vc_preset_bank(preset_choice),first=vc_preset_bank_first(bank);
  unsigned end=bank+1<vc_preset_bank_count()?vc_preset_bank_first(bank+1):vc_preset_count();
  preset_choice=preset_choice>=vc_preset_count()?first:(unsigned)clamp((int)preset_choice+d,(int)first,(int)end-1);
 }
 vc_post(8,preset_choice&255,preset_choice>>8);redraw=1;
 }
#ifdef VC_ACCESS_UI
 else if(k==EN_ALGO){
  int next=((int)access_page_index()+d)%(int)ACCESS_PAGE_COUNT;
  if(next<0)next+=ACCESS_PAGE_COUNT;
  page=access_pages[next].id;redraw=1;
 }
#else
 else if(k==EN_SELECT){row^=1;redraw=1;}
 else if(k==EN_ALGO){page=clamp(page+d,0,PAGE_LAST);if(page==PAGE_STEREO||page>=PAGE_NOISE)row=0;redraw=1;}
#endif
#ifdef VC_BAND_CACHE
 else if(page==PAGE_USER){
  if(k==EN_K1&&!user_job){user_slot=(unsigned)clamp((int)user_slot+d,0,15);user_status=user_bank_bad?6:0;}
  else if(k==EN_K2){
   if(user_job||wave_state)user_status=4;
   else if(user_bank_bad)user_status=6;
   else if(vc_patch_bank_get(&user_recall,user_bank,sizeof(user_bank),user_slot)){user_job=3;vc_post(10,0,0);}
   else user_status=3;
  }else if((k==EN_K3||k==EN_K4)&&!user_job)vc_user_edit_name(k,d);
  redraw=1;
 }
#endif
#ifdef VC_ACCESS_UI
 else if(k>=EN_K1&&k<=EN_K4){
  if(page==ACCESS_CACHE_TUNE&&k==EN_K1)band_budget=(unsigned)clamp((int)band_budget+d*256,256,8192);
  else if(access_profile_page()&&k==EN_K2){
   band_work_peak_ms=0;perf_reset();queue_lost=0;vc_post(5,0,0);
  }else{
   int id=page==PAGE_PRESETS&&k==EN_K4?VC_PATCH_VOLUME:access_pages[access_page_index()].control[k-EN_K1];
   if(id<0)continue;
   values[id]=clamp(values[id]+d,minparam(id),maxparam(id));vc_post(3,id,values[id]);
  }
  redraw=1;
 }
 }
#else
 else if(k>=EN_K1&&k<=EN_K4&&page!=PAGE_PRESETS&&(page<PAGE_PERF||page>=PAGE_NOISE)){int id=pages[page==PAGE_BOOST?23:page==PAGE_UNISON?22:page==PAGE_EQ?21:page==PAGE_DIST?20:page==PAGE_LFO_SHARED?19:page==PAGE_PITCH?18:page==PAGE_AMP_RESPONSE?17:page==PAGE_VEL_OSC?16:page==PAGE_VELOCITY?15:page==PAGE_SAT?14:page==PAGE_OSC3?13:page==PAGE_LFO3?12:page==PAGE_LFO2_OSC?11:page==PAGE_LFO2?10:page==PAGE_LFO_TIMING?9:page==PAGE_LFO_FILTER?8:page==PAGE_LFO1?7:page==PAGE_FM?6:page==PAGE_NOISE?5:page][row*4+k-EN_K1];if(id<0)continue;values[id]=clamp(values[id]+d,minparam(id),maxparam(id));vc_post(3,id,values[id]);redraw=1;}
 #ifdef VC_BAND_CACHE
 else if(page==PAGE_PROFILE&&k==EN_K1){band_budget=(unsigned)clamp((int)band_budget+d*256,256,8192);redraw=1;}
#else
 else if(page==PAGE_PROFILE&&k==EN_K1){profile_choice=clamp(profile_choice+d,0,5);vc_post(6,profile_choice,0);perf_reset();redraw=1;}
#endif
 else if(page>=PAGE_PERF&&k==EN_K2){
#ifdef VC_BAND_CACHE
 band_work_peak_ms=0;
#endif
 perf_reset();queue_lost=0;vc_post(5,0,0);}}
#endif
#ifndef VC_BAND_CACHE
 if(page==PAGE_PROFILE&&previous_page!=PAGE_PROFILE){profile_choice=0;vc_post(6,0,0);perf_reset();}
 if(page!=PAGE_PROFILE&&previous_page==PAGE_PROFILE)vc_post(7,0,0);
#endif
#ifdef VC_ACCESS_UI
 last_keys=keys;last_buttons=btn;plat_glow(1);
 unsigned shortcut=access_pages[access_page_index()].shortcut;
 plat_leds(1u<<shortcut,keys);
#else
 last_keys=keys;last_buttons=btn;plat_glow(1);plat_leds((1u<<(page==PAGE_OSC?B_HOME:page==PAGE_FILTER?B_EDIT:page==PAGE_ENV?B_ENV:(page==PAGE_FMOD||page==PAGE_LFO1||page==PAGE_LFO_FILTER||page==PAGE_LFO_TIMING||page==PAGE_LFO2||page==PAGE_LFO2_OSC||page==PAGE_LFO3||page==PAGE_LFO_SHARED)?B_LFO:(page==PAGE_STEREO||page>=PAGE_NOISE)?B_FX:B_GLO)),keys);
#endif
}
void app_input_only(void){vc_input();}
static void vc_num(char *t,unsigned v){char r[12];int n=0;do{r[n++]=(char)('0'+v%10);v/=10;}while(v);int j=0;while(n)t[j++]=r[--n];t[j]=0;}
static void vc_copy(char *d,const char *s){while((*d++=*s++)){} }
static void vc_value(int id,char *t){
 if(id==VC_TRANSPOSE||id==VC_BEND_UP||id==VC_BEND_DOWN||id==VC_OSC3_TUNE||id==VC_TUNE1||id==VC_TUNE2||id==VC_KEYFOLLOW1||id==VC_KEYFOLLOW2){
  int value=(int)values[id]-64;
  if(id==VC_KEYFOLLOW1||id==VC_KEYFOLLOW2)value=value*100/32;
  t[0]=value<0?'-':'+';vc_num(t+1,(unsigned)(value<0?-value:value));
 }
 else if(id==VC_LFO_CLOCK||id==VC_LFO2_CLOCK||id==VC_LFO3_CLOCK){static const char *names[]={"OFF","1/64","1/32","1/16","1/8","1/4","1/2","1/32 D","1/16 D","1/8 D","1/4 D","1/16 T","1/8 T","1/4 T","1/2 T","1/1 T","1/2 D","1 BAR","2 BARS","4 BARS","8 BARS","16 BARS"};vc_copy(t,names[values[id]]);}
 else if(id==VC_LFO3_DEST){static const char *names[]={"PITCH 1","PITCH 1+2","PITCH 2","PWM 1","PWM 1+2","PWM 2"};vc_copy(t,names[values[id]]);}
 else if(id==VC_LFO3_ENABLE)vc_copy(t,values[id]?"ON":"OFF");
 else if(id==VC_LFO_GLOBAL||id==VC_LFO2_GLOBAL||id==VC_LFO3_GLOBAL)vc_copy(t,values[id]?"GLOBAL":"VOICE");
 else if(id==VC_KEY_MODE){static const char *const names[]={"POLY","MONO 1","MONO 2","MONO 3","MONO 4","HOLD"};vc_copy(t,names[values[id]]);}
 else if(id==VC_AMP_VELOCITY&&values[id]==128)vc_copy(t,"LINEAR");
 else if(id==VC_BEND_SCALE)vc_copy(t,values[id]?"CURVED":"LINEAR");
 else if(id==VC_TEMPO)vc_num(t,63+values[id]);
 else if(id==VC_LFO_SHAPE||id==VC_LFO2_SHAPE||id==VC_LFO3_SHAPE){static const char *names[]={"SINE","TRIANGLE","SAW","SQUARE","S & H","S & G"};if(values[id]<6)vc_copy(t,names[values[id]]);else{vc_copy(t,"WAVE ");vc_num(t+5,values[id]-4);}}
 else if(id==VC_LFO_MODE||id==VC_LFO2_MODE){static const char *names[]={"OFF","CYCLE","ONE SHOT"};vc_copy(t,names[values[id]]);}
 else if(id==VC_LFO_TRIGGER||id==VC_LFO2_TRIGGER){if(!values[id])vc_copy(t,"FREE");else{vc_num(t,(values[id]-1)*360/128);unsigned n=0;while(t[n])n++;vc_copy(t+n," DEG");}}
 else if(id==VC_SAT_CURVE){static const char *names[]={"OFF","LIGHT","SOFT","MIDDLE","HARD","DIGITAL","SHAPER","RECTIFIER","BIT REDUCER","RATE REDUCE","RATE+FLW","LOWPASS","LOW+FLW","HIGHPASS","HIGH+FLW"};vc_copy(t,names[values[id]]);}
 else if(id==VC_OSC3_MODE){static const char *names[]={"OFF","SLAVE","SAW","PULSE"};if(values[id]<4)vc_copy(t,names[values[id]]);else{vc_copy(t,"WAVE ");vc_num(t+5,values[id]-4);}}
 else if(id==VC_FM_MODE){static const char *names[]={"POS TRI","TRIANGLE","OSC 1","NOISE"};vc_copy(t,names[values[id]]);}
 else if(id==VC_FM_ENV||id==VC_FM_VELOCITY||id==VC_OSC2_ENV||id==VC_LFO_CONTOUR||id==VC_LFO_OSC1||id==VC_LFO_OSC2||id==VC_LFO_PW||id==VC_LFO_RESONANCE||id==VC_LFO_FILTER_GAIN||id==VC_LFO2_CONTOUR||id==VC_LFO2_CUTOFF1||id==VC_LFO2_CUTOFF2||id==VC_LFO2_SHAPE_AMOUNT||id==VC_LFO2_FM_AMOUNT||id==VC_LFO2_PAN){int x=(int)values[id]-64;t[0]=x<0?'-':'+';vc_num(t+1,(unsigned)(x<0?-x:x));}
 else if(id==VC_INIT_PHASE){
  unsigned phase=values[id];
  if(!phase)vc_copy(t,"RANDOM");else if(phase==128)vc_copy(t,"RESET");
  else{vc_num(t,phase*360/128);unsigned n=0;while(t[n])n++;vc_copy(t+n," DEG");}
 }
 else if(id==VC_UNISON_MODE){if(!values[id])vc_copy(t,"OFF");else vc_num(t,values[id]+1);}
 else if(id==VC_SUB_SHAPE)vc_copy(t,values[id]?"TRIANGLE":"SQUARE");
 else if(id==VC_FILTER_MODE||id==VC_FILTER_MODE2){static const char *names[]={"LP","HP","BP","NOTCH"};vc_copy(t,names[values[id]]);}
 else if(id==VC_ROUTING){static const char *names[]={"SER 4","SER 6","PARALLEL","SPLIT"};vc_copy(t,names[values[id]]);}
 else if(id==VC_EQ_ENABLE||id==VC_SYNC||id==VC_CUTOFF_LINK)vc_copy(t,values[id]?"ON":"OFF");
 else if(id==VC_F_POLARITY1||id==VC_F_POLARITY2)vc_copy(t,values[id]?"POS":"NEG");
 else if(id==VC_AMP_SLOPE||id==VC_F_SLOPE){if(values[id]==64)vc_copy(t,"HOLD");else{vc_copy(t,values[id]<64?"FALL ":"RISE ");vc_num(t+5,values[id]);}}
 else vc_num(t,values[id]);
}
#ifdef VC_ACCESS_UI
#include "access_draw.inc"
#else
static void vc_draw(void){
 char t[40];cv_begin(240,32,C_BLACK);cv_text(8,5,&FONT_B,
#ifdef VC_BAND_CACHE
 user_loaded?user_active.name:
#endif
 preset_loaded<vc_preset_count()?vc_preset_name(preset_loaded):"VIROOOS",C_HI);cv_text(145,9,&FONT_XS,APP_TAGLINE,C_GRAY);cv_blit(0,0);
 cv_begin(240,24,C_BLACK);cv_text(8,3,&FONT_XS,page==PAGE_PROFILE?
#ifdef VC_BAND_CACHE
 "WAVE CACHE (K1 BUDGET)"
#else
 profile_names[profile_choice]
#endif
 :page_names[page],C_WHITE);cv_blit(0,32);
 perf_stats_t ps;if(page>=PAGE_PERF)perf_read(&ps);
 for(int y=0;y<4;y++)for(int x=0;x<2;x++){int index=y*2+x;cv_begin(120,34,C_BLACK);
#ifdef VC_BAND_CACHE
 if(page==PAGE_USER){
  const char *names[]={"USER SLOT","STATE","STORED NAME","EDIT NAME","K1: SLOT","K2: RECALL","K3: POSITION","K4: LETTER"};
  cv_text(2,5,&FONT_XS,names[index],C_GRAY);t[0]=0;
  if(index==0)vc_num(t,user_slot+1);
  else if(index==1)vc_copy(t,user_job?"WORKING":user_status_names[user_status]);
  else if(index==2){unsigned mask=(unsigned)user_bank[12]|(unsigned)user_bank[13]<<8;if(user_bank_bad)vc_copy(t,"INVALID");else if(mask&(1u<<user_slot)){memcpy(t,user_bank+16+user_slot*VC_PATCH_BYTES+16,10);t[10]=0;}else vc_copy(t,"EMPTY");}
  else if(index==3)vc_copy(t,user_name_edit?user_name:user_loaded?user_active.name:vc_preset_name(preset_loaded));
  else if(index==6)vc_num(t,user_name_pos+1);
  else if(index==7){const char *name=user_name_edit?user_name:user_loaded?user_active.name:vc_preset_name(preset_loaded);unsigned length=0;while(length<10&&name[length])length++;t[0]=user_name_pos<length?name[user_name_pos]:' ';t[1]=0;}
  cv_text(3,17,&FONT_B,t,C_WHITE);cv_blit(x*120,60+y*34);continue;
 }
#endif
 if(page==PAGE_PRESETS){const char *names[]={"BANK","PROGRAM","STATUS","COUNT","K1: BANK","K2: PROGRAM","K3: INIT","ARP REMOVED"};cv_text(2,5,&FONT_XS,names[index],C_GRAY);if(index==0){vc_copy(t,vc_preset_bank_label(vc_preset_bank(preset_loaded)));}else if(index==1)vc_num(t,vc_preset_program(preset_loaded));else if(index==2)vc_copy(t,preset_loaded>=vc_preset_count()?"INIT":vc_preset_partial(preset_loaded)?"PARTIAL":vc_preset_wave_cache(preset_loaded)?"CACHE":"MAPPED");else if(index==3)vc_num(t,vc_preset_count());else t[0]=0;cv_text(3,17,&FONT_B,t,C_WHITE);cv_blit(x*120,60+y*34);continue;}
 if(page<PAGE_PERF||page>=PAGE_NOISE){int id=pages[page==PAGE_BOOST?23:page==PAGE_UNISON?22:page==PAGE_EQ?21:page==PAGE_DIST?20:page==PAGE_LFO_SHARED?19:page==PAGE_PITCH?18:page==PAGE_AMP_RESPONSE?17:page==PAGE_VEL_OSC?16:page==PAGE_VELOCITY?15:page==PAGE_SAT?14:page==PAGE_OSC3?13:page==PAGE_LFO3?12:page==PAGE_LFO2_OSC?11:page==PAGE_LFO2?10:page==PAGE_LFO_TIMING?9:page==PAGE_LFO_FILTER?8:page==PAGE_LFO1?7:page==PAGE_FM?6:page==PAGE_NOISE?5:page][index];if(id<0){cv_blit(x*120,60+y*34);continue;}cv_text(2,5,&FONT_XS,labels[id],row==index/4?C_HI:C_GRAY);vc_value(id,t);cv_text(3,17,&FONT_B,t,C_WHITE);cv_rect(2,32,112*(values[id]-minparam(id))/(maxparam(id)-minparam(id)),2,C_HI);}
#ifdef VC_BAND_CACHE
 else if(page==PAGE_PROFILE){unsigned val[8];vc_band_device_stats(val);val[6]=band_budget;val[7]=band_work_peak_ms;const char *names[]={"READY /32","WAIT /12","BUILT","CANCELLED","FAILURES","BUILD %","BUDGET","WORK MAX MS"};cv_text(2,5,&FONT_XS,names[index],C_GRAY);vc_num(t,val[index]);cv_text(3,17,&FONT_B,t,C_WHITE);}
#else
 else if(page==PAGE_PROFILE){const char *names[]={"CPU %","PEAK %","LAST US","CPU MHZ","XRUNS","DEADLINE","VOICES","BLOCK"};unsigned val[]={ps.audio_pct,ps.peak_pct,ps.last_us,ps.cpu_mhz,ps.dropouts,ps.deadline_misses,active_voices,APP_BLOCK};cv_text(2,5,&FONT_XS,names[index],C_GRAY);vc_num(t,val[index]);cv_text(3,17,&FONT_B,t,C_WHITE);}
#endif
#ifdef VC_DUAL_RENDER
 else if(page==PAGE_PERF&&row){const char *names[]={"DUAL ACTIVE","CORE STATUS","CPU1 US","CPU0 WAIT US","JOBS","FAULTS","CPU1 PEAK US","WAIT PEAK US"};unsigned val[]={ps.dual,ps.status,ps.worker_us,ps.wait_us,ps.jobs,ps.faults,ps.worker_peak_us,ps.wait_peak_us};cv_text(2,5,&FONT_XS,names[index],C_GRAY);vc_num(t,val[index]);cv_text(3,17,&FONT_B,t,C_WHITE);}
#endif
 else {const char *names[]={"CPU %","PEAK %","XRUNS","DEADLINE","VOICES","QUEUE","CLIPPED","BLOCK"};unsigned val[]={ps.audio_pct,ps.peak_pct,ps.dropouts,ps.deadline_misses,active_voices,queue_lost,clipped_samples,APP_BLOCK};cv_text(2,5,&FONT_XS,names[index],C_GRAY);vc_num(t,val[index]);cv_text(3,17,&FONT_B,t,C_WHITE);}
 cv_blit(x*120,60+y*34);}
 const char *footer=page==PAGE_PROFILE?"K1: STAGE  K2: RESET":"SEL: BANK  ALGO: PAGE";
 const char *hint=page==PAGE_PROFILE?"MUTED  PLAY: EXIT":"PLAY: PANIC   4 VOICES";
#ifdef VC_BAND_CACHE
 if(page==PAGE_PROFILE){footer="K1: BUDGET  K2: RESET";hint="LIVE CACHE MONITOR";}
 if(page==PAGE_USER)footer="SAVE: STORE  ALGO: PAGE";
#ifdef VC_DUAL_RENDER
 if(page==PAGE_PERF)hint="SEL: CORE STATS   K2: RESET";
#endif
#endif
 if(wave_state==3){
  const char *stage=wave_error_stage?wave_error_stage:"UNKNOWN";
  unsigned end=0;while(stage[end]){t[end]=stage[end];end++;}t[end++]=' ';
  vc_num(t+end,wave_error_index);
  footer=t;
 }else if(wave_embedded){
  const char digits[]="0123456789ABCDEF";
  uint32_t id=plat_flash_jedec();
  vc_copy(t,"EMBEDDED WAVES / FLASH ");
  for(unsigned i=0;i<6;i++)t[23+i]=digits[(id>>(20-4*i))&15u];
  t[29]=0;footer=t;
 }
 cv_begin(240,32,C_BLACK);cv_text(8,2,&FONT_XS,footer,C_GRAY);cv_text(8,17,&FONT_XS,wave_state==3?"WAVE DATA ERROR":wave_state?"LOADING WAVES - MUTED":hint,C_GRAY);cv_blit(0,204);
}
#endif
#ifdef VC_BAND_CACHE
static void vc_user_reply(unsigned slot,unsigned status){uint8_t frame[12],value=(uint8_t)status;unsigned n=vc_patch_sysex_encode(frame,sizeof(frame),4,slot,&value,1);if(n)plat_sysex_send(frame,n);}
int app_sysex(const uint8_t *p,uint32_t n){
 if(n<4||p[0]!=0x7d||p[1]!=0x56||p[2]!=0x49||p[3]!=0x52)return 0;
 unsigned cmd,slot,bytes;uint8_t raw[VC_PATCH_BYTES];
 if(!vc_patch_sysex_decode(p,n,&cmd,&slot,raw,&bytes)){vc_user_reply(n>6&&p[6]<16?p[6]:0,3);return 1;}
 if(cmd==2||cmd==4)return 1; /* Responses are never treated as writes. */
 if(user_job||wave_state){vc_user_reply(slot,1);return 1;}
 if(user_bank_bad){vc_user_reply(slot,3);return 1;}
 if(cmd==1){
  VCPatchRecord record;
  if(!vc_patch_bank_get(&record,user_bank,sizeof(user_bank),slot)){vc_user_reply(slot,2);return 1;}
  uint8_t frame[VC_PATCH_WIRE_MAX];vc_patch_encode(raw,sizeof(raw),&record);
  unsigned count=vc_patch_sysex_encode(frame,sizeof(frame),2,slot,raw,sizeof(raw));if(count)plat_sysex_send(frame,count);
 }else if(cmd==3){
  if(!vc_patch_decode(&user_recall,raw,bytes)){vc_user_reply(slot,3);return 1;}
  user_slot=slot;user_remote_pending=1;user_job=3;vc_post(11,0,0);
 }
 return 1;
}
#endif
void app_ui_init(void){
 touch_on(1);
#if defined(VC_ACCESS_UI) && defined(PLAT_HOST)
 app_splash();
#elif !defined(VC_ACCESS_UI)
 lcd_fill(0,0,240,240,C_BLACK);
#endif
 redraw=1;
}
void app_frame(void){if(event_read==event_write){
 unsigned selection=preset_requested;
 if(selection==NO_PRESET_REQUEST)selection=preset_loaded;
 if(preset_choice!=selection){preset_choice=selection;redraw=1;}
}
vc_input();vc_preset_background();
#ifdef VC_BAND_CACHE
vc_user_background();
#endif
vc_wave_background();
#ifdef VC_BAND_CACHE
if(!wave_state){unsigned before=plat_ms();vc_band_device_service(band_budget);unsigned elapsed=plat_ms()-before;if(elapsed>band_work_peak_ms)band_work_peak_ms=elapsed;}
#endif
if(ui_wave_state!=wave_state){ui_wave_state=wave_state;redraw=1;}if(event_read==event_write){for(int i=0;i<VC_PARAMS;i++)if(!(wave_state&&(i==VC_WAVE1||i==VC_WAVE2||i==VC_OSC3_MODE||i==VC_LFO_SHAPE||i==VC_LFO2_SHAPE||i==VC_LFO3_SHAPE))&&values[i]!=synth.param[i]){values[i]=synth.param[i];redraw=1;}}
#ifdef VC_ACCESS_UI
 int refresh=access_profile_page()||wave_state;
#else
 int refresh=page>=PAGE_PERF||wave_state;
#endif
 if(redraw||(refresh&&plat_ms()-last_draw>250)){redraw=0;last_draw=plat_ms();vc_draw();}
}
#ifdef PLAT_HOST
int app_expect(const char *s,int *v){
#ifdef VC_BAND_CACHE
if(!strcmp(s,"user_name_first")){*v=(unsigned char)user_active.name[0];return 1;}
if(!strcmp(s,"user_matches_slot")){VCPatchRecord saved;*v=user_loaded&&vc_patch_bank_get(&saved,user_bank,sizeof(user_bank),user_slot)&&!memcmp(saved.params,synth.param,VC_PARAMS)&&!memcmp(&saved.matrix,&user_active.matrix,sizeof(VCMatrix));return 1;}
if(!strcmp(s,"user_job")){*v=user_job;return 1;}if(!strcmp(s,"user_status")){*v=user_status;return 1;}if(!strcmp(s,"user_loaded")){*v=user_loaded;return 1;}if(!strcmp(s,"user_slot")){*v=user_slot;return 1;}
#endif

 if(!strcmp(s,"matrix_active")){*v=synth.matrix!=0;return 1;}
 if(!strcmp(s,"matrix_wheel")){*v=(int)(synth.controllers[3]*127+.5f);return 1;}
 if(!strcmp(s,"matrix_pressure")){*v=(int)(synth.controllers[2]*127+.5f);return 1;}
 if(!strcmp(s,"matrix_cutoff_delta")||!strcmp(s,"matrix_env_delta")||!strcmp(s,"matrix_key_delta")){float source[28],d[VC_MATRIX_FILTERS];for(unsigned i=0;i<28;i++)source[i]=synth.controllers[i];if(synth.matrix){vc_matrix_filters(synth.matrix,source,d);unsigned index=!strcmp(s,"matrix_env_delta")?4:!strcmp(s,"matrix_key_delta")?6:0;*v=(int)(d[index]*1000);}else *v=0;return 1;}
if(!strcmp(s,"flash_tables")){*v=vc_wave_flash_hits();return 1;}
if(!strcmp(s,"wave_embedded")){*v=wave_embedded;return 1;}
if(!strcmp(s,"preset_index")){*v=preset_loaded;return 1;}if(!strcmp(s,"preset_program")){*v=vc_preset_program(preset_loaded);return 1;}if(!strcmp(s,"preset_bank")){*v=vc_preset_bank(preset_loaded);return 1;}if(!strcmp(s,"lfo3_global")){*v=synth.param[VC_LFO3_GLOBAL];return 1;}if(!strcmp(s,"eq_enabled")){*v=synth.param[VC_EQ_ENABLE];return 1;}if(!strcmp(s,"eq_low_gain")){*v=synth.param[VC_EQ_LOW_GAIN];return 1;}if(!strcmp(s,"eq_mid_gain")){*v=synth.param[VC_EQ_MID_GAIN];return 1;}if(!strcmp(s,"eq_mid_q")){*v=synth.param[VC_EQ_MID_Q];return 1;}if(!strcmp(s,"eq_high_gain")){*v=synth.param[VC_EQ_HIGH_GAIN];return 1;}if(!strcmp(s,"dist_mode")){*v=synth.param[VC_DIST_MODE];return 1;}if(!strcmp(s,"dist_level")){*v=synth.param[VC_DIST_LEVEL];return 1;}if(!strcmp(s,"lfo3_shape")){*v=synth.param[VC_LFO3_SHAPE];return 1;}if(!strcmp(s,"lfo_global"))*v=synth.param[VC_LFO_GLOBAL];else if(!strcmp(s,"lfo2_global"))*v=synth.param[VC_LFO2_GLOBAL];else if(!strcmp(s,"ui_lfo_global"))*v=values[VC_LFO_GLOBAL];else if(!strcmp(s,"ui_lfo2_global"))*v=values[VC_LFO2_GLOBAL];else if(!strcmp(s,"ui_lfo_mode"))*v=values[VC_LFO_MODE];else if(!strcmp(s,"ui_lfo2_mode"))*v=values[VC_LFO2_MODE];else if(!strcmp(s,"lfo_extra_verified"))*v=0;else if(!strcmp(s,"settings_sentinel"))*v=plat_data(0xDC000u)[0];else if(!strcmp(s,"lfo_extra_ready"))*v=0;else if(!strcmp(s,"lfo_extra_written"))*v=0;else if(!strcmp(s,"lfo_extra_index"))*v=0;else if(!strcmp(s,"key_mode"))*v=synth.param[VC_KEY_MODE];else if(!strcmp(s,"mono_note"))*v=synth.voice[0].note;else if(!strcmp(s,"mono_held"))*v=synth.mono_count;else if(!strcmp(s,"ui_key_mode"))*v=values[VC_KEY_MODE];else if(!strcmp(s,"lfo_rate"))*v=synth.param[VC_LFO_RATE];else if(!strcmp(s,"ui_lfo_rate"))*v=values[VC_LFO_RATE];else if(!strcmp(s,"lfo2_rate"))*v=synth.param[VC_LFO2_RATE];else if(!strcmp(s,"ui_lfo2_rate"))*v=values[VC_LFO2_RATE];else if(!strcmp(s,"lfo2_contour"))*v=synth.param[VC_LFO2_CONTOUR];else if(!strcmp(s,"ui_wave1"))*v=values[VC_WAVE1];else if(!strcmp(s,"ui_wave2"))*v=values[VC_WAVE2];else if(!strcmp(s,"desired_wave1"))*v=wave_desired[0];else if(!strcmp(s,"desired_wave2"))*v=wave_desired[1];else if(!strcmp(s,"desired_wave3"))*v=wave_desired[2];else if(!strcmp(s,"cached_wave3"))*v=vc_wave_cached(2);else if(!strcmp(s,"shape1"))*v=synth.param[VC_SHAPE1];else if(!strcmp(s,"ui_shape1"))*v=values[VC_SHAPE1];else if(!strcmp(s,"attack"))*v=synth.param[VC_ATTACK];else if(!strcmp(s,"ui_attack"))*v=values[VC_ATTACK];else if(!strcmp(s,"resonance"))*v=synth.param[VC_RESONANCE];else if(!strcmp(s,"ui_cutoff"))*v=values[VC_CUTOFF];else if(!strcmp(s,"ui_patch_volume"))*v=values[VC_PATCH_VOLUME];else if(!strcmp(s,"ui_pan"))*v=values[VC_PAN];else if(!strcmp(s,"ui_portamento"))*v=values[VC_PORTAMENTO];else if(!strcmp(s,"pan"))*v=synth.param[VC_PAN];else if(!strcmp(s,"patch_volume"))*v=synth.param[VC_PATCH_VOLUME];else if(!strcmp(s,"portamento"))*v=synth.param[VC_PORTAMENTO];else if(!strcmp(s,"transpose"))*v=synth.param[VC_TRANSPOSE];else if(!strcmp(s,"bend_up"))*v=synth.param[VC_BEND_UP];else if(!strcmp(s,"bend_down"))*v=synth.param[VC_BEND_DOWN];else if(!strcmp(s,"bend_curve"))*v=synth.param[VC_BEND_SCALE];else if(!strcmp(s,"amp_velocity"))*v=synth.param[VC_AMP_VELOCITY];else if(!strcmp(s,"vel_shape1"))*v=synth.param[VC_VEL_SHAPE1];else if(!strcmp(s,"vel_pw"))*v=synth.param[VC_VEL_PW];else if(!strcmp(s,"vel_pan"))*v=synth.param[VC_VEL_PAN];else if(!strcmp(s,"vel_env1"))*v=synth.param[VC_VEL_ENV1];else if(!strcmp(s,"vel_res2"))*v=synth.param[VC_VEL_RES2];else if(!strcmp(s,"punch"))*v=synth.param[VC_PUNCH];else if(!strcmp(s,"sat_curve"))*v=synth.param[VC_SAT_CURVE];else if((!strcmp(s,"sat_drive")||!strcmp(s,"osc_volume")))*v=synth.param[VC_SAT_DRIVE];else if(!strcmp(s,"osc3_mode"))*v=synth.param[VC_OSC3_MODE];else if(!strcmp(s,"osc3_volume"))*v=synth.param[VC_OSC3_VOLUME];else if(!strcmp(s,"lfo3_enable"))*v=synth.param[VC_LFO3_ENABLE];else if(!strcmp(s,"lfo3_amount"))*v=synth.param[VC_LFO3_AMOUNT];else if(!strcmp(s,"lfo3_dest"))*v=synth.param[VC_LFO3_DEST];else if(!strcmp(s,"lfo2_pan"))*v=synth.param[VC_LFO2_PAN];else if(!strcmp(s,"lfo2_shape_amount"))*v=synth.param[VC_LFO2_SHAPE_AMOUNT];else if(!strcmp(s,"lfo2_fm_amount"))*v=synth.param[VC_LFO2_FM_AMOUNT];else if(!strcmp(s,"lfo2_clock"))*v=synth.param[VC_LFO2_CLOCK];else if(!strcmp(s,"lfo2_mode"))*v=synth.param[VC_LFO2_MODE];else if(!strcmp(s,"lfo2_cutoff1"))*v=synth.param[VC_LFO2_CUTOFF1];else if(!strcmp(s,"lfo2_shape"))*v=synth.param[VC_LFO2_SHAPE];else if(!strcmp(s,"lfo_clock"))*v=synth.param[VC_LFO_CLOCK];else if(!strcmp(s,"tempo"))*v=63+synth.param[VC_TEMPO];else if(!strcmp(s,"lfo_keytrack"))*v=synth.param[VC_LFO_KEYTRACK];else if(!strcmp(s,"lfo_resonance"))*v=synth.param[VC_LFO_RESONANCE];else if(!strcmp(s,"lfo_filter_gain"))*v=synth.param[VC_LFO_FILTER_GAIN];else if(!strcmp(s,"lfo_mode"))*v=synth.param[VC_LFO_MODE];else if(!strcmp(s,"lfo_shape"))*v=synth.param[VC_LFO_SHAPE];else if(!strcmp(s,"lfo_osc1"))*v=synth.param[VC_LFO_OSC1];else if(!strcmp(s,"init_phase"))*v=synth.param[VC_INIT_PHASE];else if(!strcmp(s,"osc2_env"))*v=synth.param[VC_OSC2_ENV];else if(!strcmp(s,"fm_amount"))*v=synth.param[VC_FM_AMOUNT];else if(!strcmp(s,"fm_mode"))*v=synth.param[VC_FM_MODE];else if(!strcmp(s,"fm_env"))*v=synth.param[VC_FM_ENV];else if(!strcmp(s,"wave_install_bytes"))*v=0;else if(!strcmp(s,"wave_state"))*v=(int)wave_state;else if(!strcmp(s,"wave_progress"))*v=(int)vc_wave_progress();else if(!strcmp(s,"boost_level"))*v=synth.param[VC_BOOST_LEVEL];else if(!strcmp(s,"boost_tune"))*v=synth.param[VC_BOOST_TUNE];else if(!strcmp(s,"voices"))*v=active_voices;
#ifdef VC_DUAL_RENDER
else if(!strcmp(s,"shed_count"))*v=shed_count;else if(!strcmp(s,"shed_now")){app_overload();*v=0;}
#endif
else if(!strcmp(s,"page"))*v=page;else if(!strcmp(s,"row"))*v=row;else if(!strcmp(s,"cutoff"))*v=synth.param[VC_CUTOFF];else if(!strcmp(s,"f_amount1"))*v=synth.param[VC_F_AMOUNT1];else if(!strcmp(s,"routing"))*v=synth.param[VC_ROUTING];else if(!strcmp(s,"f_attack"))*v=synth.param[VC_F_ATTACK];else if(!strcmp(s,"sub_volume"))*v=synth.param[VC_SUB_VOLUME];else if(!strcmp(s,"ring_volume"))*v=synth.param[VC_RING_VOLUME];else if(!strcmp(s,"noise_volume"))*v=synth.param[VC_NOISE_VOLUME];else if(!strcmp(s,"noise_color"))*v=synth.param[VC_NOISE_COLOR];else if(!strcmp(s,"wave1"))*v=synth.param[VC_WAVE1];else if(!strcmp(s,"wave2"))*v=synth.param[VC_WAVE2];else if(!strcmp(s,"width1"))*v=synth.param[VC_PW1];else if(!strcmp(s,"width2"))*v=synth.param[VC_PW2];else if(!strcmp(s,"tune1"))*v=synth.param[VC_TUNE1];else if(!strcmp(s,"tune2"))*v=synth.param[VC_TUNE2];else if(!strcmp(s,"keyfollow1"))*v=synth.param[VC_KEYFOLLOW1];else if(!strcmp(s,"profile_mode"))*v=-1;else if(!strcmp(s,"queue_lost"))*v=queue_lost;else return 0;return 1;}
#endif

#define VC_LFO_MAX_SHAPE 67u
#ifndef VIROOOS_H
#define VIROOOS_H
#include <stdint.h>
#ifdef VC_MATRIX
#include "matrix.h"
#endif
#ifdef VC_POST_FX
#include "distortion.h"
#include "eq.h"
#endif
#include "voices.h"
#ifdef VC_DYNAMIC_WAVES
#define VC_WAVES 64
#else
#define VC_WAVES 4
#endif
#define VC_CONTROL_FRAMES 32
/* Execute target audio kernels from existing copied SRAM section. */
#ifdef PLAT_HOST
#define VC_HOT
#else
#define VC_HOT __attribute__((section(".voice_hot")))
#endif
/* Per-sample helpers. The dual-render voice kernel inlines them, so there
 * they stay in flash for the profile page and tests; the single-core
 * render calls them from SRAM, so there they stay in SRAM. */
#ifdef VC_DUAL_RENDER
#define VC_HELPER
#else
#define VC_HELPER VC_HOT
#endif

typedef struct {
    float phase,pending,wave_gain,saw_gain,pulse_gain,duty;
    int wave,band;
    const float *table;
#ifdef VC_BAND_CACHE
    float table_correction;
    unsigned table_ramp;
#endif
    const float *const *tables;
} VCOsc;
typedef struct {
    float s1,s2,s3,s4,g,k,gain,inverse;
    float cutoff,resonance,sample_rate;
    int mode,poles,configured;
} VCFilter;
typedef struct {
    float level,attack_inc,decay_coef,sustain,release_coef;
    float fall_coef,rise_inc;
    int stage,slope;
} VCEnvelope;
typedef struct {
 float sample_rate,phase,rate,contour,cycles,start,current,next,value;
 uint32_t rng;
 unsigned shape,env_mode,done;
} VCLfo;
void vc_lfo_init(VCLfo*,float,uint32_t);
void vc_lfo_config(VCLfo*,float,unsigned,float,unsigned);
void vc_lfo_trigger(VCLfo*,unsigned);
float vc_lfo_deviation(VCLfo*);
float vc_lfo_rate(float,int,unsigned,float,unsigned,unsigned);
VC_HOT float vc_lfo_tick(VCLfo*,unsigned);
VC_HOT float vc_lfo_shape(const VCLfo*,float);
typedef struct {
 const float *lo,*hi;
 float mix,hold,counter,period,lp,coef,sample_rate,semitones;
 float level;
 unsigned curve,configured;
} VCSaturation;
void vc_sat_setup(VCSaturation*,float,unsigned,float,float);
VC_HELPER float vc_sat_next(VCSaturation*,float);
typedef struct {float phase;} VCSub;
typedef struct {float gain,coef,lp,sample_rate;int color,configured;uint32_t rng;} VCNoise;
typedef struct {
    float unison_side,unison_cents,unison_lfo_offset;
    unsigned unison_odd;
    VCSub sub;
    VCLfo lfo,lfo2,lfo3;
    float lfo3_deviation,lfo3_mod;
    uint32_t lfo3_elapsed_frames;
    float lfo2_deviation,lfo2_mod,lfo2_shape_last;
    float pan_now[4],pan_target[4],pan_step[4];
    unsigned pan_initialized,pan_active,pan_remaining,pan_stop;
    float lfo_filter_gain,lfo_deviation,lfo_mod,pitch1_base_inc,pitch1_last,pw_last[2];
    VCNoise noise;
    VCOsc osc[2],osc3;
    float inc3,osc3_base_inc,osc3_pitch_last,osc3_gain;
    VCFilter filter[2];
    VCSaturation saturation;
    float velocity_shape[2],velocity_pw,velocity_pan;
    float filter_env_amount[2],filter_res_velocity[2];
    float source_gain[6];
    float punch_gain;
    unsigned punch_at;
    VCEnvelope env,filter_env;
    float inc[2],velocity,pitch,glide_error;
#ifdef VC_MATRIX
    float matrix_delta[VC_MATRIX_FILTERS];
    float matrix_sources[VC_MATRIX_ROUTES];
    unsigned matrix_delta_valid;
    float release_velocity;
    float envelope_values[10];
    float lfo_rate_values[3];
    float portamento_value,portamento_coef;
    float osc3_matrix_cents,osc2_matrix_detune,key_matrix_cents[2],transpose_matrix;
    float amp_velocity_amount,amp_velocity_gain;
    float filter_mix[2];
    unsigned filter_mix_after;
#endif
    float amp_gain,amp_target,amp_step;
    unsigned amp_remaining;
    float fm_depth,fm_scale,fm_noise_sample;
    float pitch2_base_inc,pitch2_env_last;
    int note,held,control_remaining;
    unsigned generated_glide_left;
    float generated_glide_delta;
    uint32_t age;
} VCVoice;
enum {
    VC_SHAPE1,VC_SHAPE2,VC_BALANCE,VC_DETUNE,
    VC_CUTOFF,VC_RESONANCE,VC_FILTER_MODE,VC_ROUTING,
    VC_ATTACK,VC_DECAY,VC_SUSTAIN,VC_RELEASE,
    VC_WAVE1,VC_WAVE2,VC_SYNC,VC_CUTOFF2,
    VC_FILTER_BALANCE,VC_FILTER_MODE2,VC_RESONANCE2,
    VC_F_ATTACK,VC_F_DECAY,VC_F_SUSTAIN,VC_F_RELEASE,
    VC_F_AMOUNT1,VC_F_AMOUNT2,VC_F_POLARITY1,VC_F_POLARITY2,
    VC_CUTOFF_LINK,VC_F_KEYFOLLOW1,VC_F_KEYFOLLOW2,VC_F_KEYBASE,
    VC_AMP_SLOPE,VC_F_SLOPE,VC_PAN,VC_SPREAD,VC_SUB_VOLUME,VC_SUB_SHAPE,VC_RING_VOLUME,VC_NOISE_VOLUME,VC_NOISE_COLOR,VC_PW1,VC_PW2,VC_TUNE1,VC_TUNE2,VC_KEYFOLLOW1,VC_KEYFOLLOW2,VC_FM_AMOUNT,VC_FM_MODE,VC_FM_ENV,VC_FM_VELOCITY,VC_OSC2_ENV,VC_INIT_PHASE,VC_LFO_RATE,VC_LFO_SHAPE,VC_LFO_CONTOUR,VC_LFO_TRIGGER,VC_LFO_OSC1,VC_LFO_OSC2,VC_LFO_PW,VC_LFO_MODE,VC_LFO_RESONANCE,VC_LFO_FILTER_GAIN,VC_LFO_CLOCK,VC_TEMPO,VC_LFO_KEYTRACK,VC_LFO2_RATE,VC_LFO2_SHAPE,VC_LFO2_CONTOUR,VC_LFO2_TRIGGER,VC_LFO2_CUTOFF1,VC_LFO2_CUTOFF2,VC_LFO2_MODE,VC_LFO2_KEYTRACK,VC_LFO2_SHAPE_AMOUNT,VC_LFO2_FM_AMOUNT,VC_LFO2_CLOCK,VC_LFO2_PAN,VC_LFO3_RATE,VC_LFO3_SHAPE,VC_LFO3_AMOUNT,VC_LFO3_DEST,VC_LFO3_FADE,VC_LFO3_KEYTRACK,VC_LFO3_CLOCK,VC_LFO3_ENABLE,VC_OSC3_MODE,VC_OSC3_VOLUME,VC_OSC3_TUNE,VC_OSC3_DETUNE,VC_SAT_CURVE,VC_SAT_DRIVE,VC_PUNCH,VC_VEL_ENV1,VC_VEL_ENV2,VC_VEL_RES1,VC_VEL_RES2,VC_VEL_SHAPE1,VC_VEL_SHAPE2,VC_VEL_PW,VC_VEL_PAN,VC_AMP_VELOCITY,VC_TRANSPOSE,VC_BEND_UP,VC_BEND_DOWN,VC_BEND_SCALE,VC_PORTAMENTO,VC_PATCH_VOLUME,VC_KEY_MODE,VC_LFO_GLOBAL,VC_LFO2_GLOBAL,VC_DIST_MODE,VC_DIST_LEVEL,VC_EQ_ENABLE,VC_EQ_LOW_GAIN,VC_EQ_LOW_FREQ,VC_EQ_MID_GAIN,VC_EQ_MID_FREQ,VC_EQ_MID_Q,VC_EQ_HIGH_GAIN,VC_EQ_HIGH_FREQ,VC_LFO3_GLOBAL,VC_UNISON_MODE,VC_UNISON_DETUNE,VC_UNISON_PHASE,VC_BOOST_LEVEL,VC_BOOST_TUNE,VC_PARAMS
};
typedef struct {
    VCVoice voice[VC_VOICES];
#ifdef VC_DUAL_RENDER
    float voice_last[VC_VOICES][2],voice_fade_from[VC_VOICES][2];
    unsigned voice_fade_left[VC_VOICES];
#endif
    VCLfo global_lfo[3];
    unsigned global_remaining;
    uint8_t param[VC_PARAMS];
#ifdef VC_MATRIX
    const VCMatrix *matrix;
    uint8_t matrix_source_ids[VC_MATRIX_ROUTES],matrix_source_count;
    float controllers[28];
#endif
    float sample_rate,bend,master,pitch_offset,glide_coef;
    float channel_volume,expression;
    int last_note;
    float mix[2],pan[4];
    int mix_after;
    uint32_t counter;
    int pedal;
    uint8_t mono_notes[128],mono_deferred[128];
    uint8_t hold_latched[16]; /* Key mode 5, note-off latch bitmap. */
    unsigned mono_count,mono_deferred_count;
#ifdef VC_POST_FX
    VCDistortion distortion;
    VCBoost boost;
    VCEQ eq;
#endif
} VCSynth;
VC_HOT void vc_pitch_env_control(VCSynth*,VCVoice*);
VC_HOT void vc_fm_control(VCSynth*,VCVoice*);
VC_HOT void vc_lfo2_osc_control(VCSynth*,VCVoice*);
VC_HOT void vc_lfo2_pan_control(VCSynth*,VCVoice*);
#ifdef VC_DUAL_RENDER
void vc_osc3_control(VCSynth*,VCVoice*);
#else
VC_HOT void vc_osc3_control(VCSynth*,VCVoice*);
#endif
VC_HELPER float vc_fm_increment(const VCVoice*,unsigned mode,float phase,float raw1);
void vc_noise_setup(VCNoise*,float sr,unsigned color);
VC_HELPER float vc_noise_next(VCNoise*);
VC_HELPER float vc_sub_next(VCSub*,float increment,int triangle);
void vc_osc_setup(VCOsc*,int wave,float shape,float pulse_width,float increment);
VC_HELPER float vc_osc_next(VCOsc*,float increment,float sync_fraction);
VC_HOT void vc_filter_setup(VCFilter*,float sample_rate,int mode,int poles,float cutoff,float resonance);
VC_HELPER float vc_filter_next(VCFilter*,float);
void vc_env_setup(VCEnvelope*,float sr,float attack,float decay,float sustain,float release);
void vc_env_slope(VCEnvelope*,float sr,unsigned value);
VC_HELPER float vc_env_next(VCEnvelope*);
void vc_route_setup(VCSynth*);
VC_HELPER void vc_mix_sources(VCSynth*,VCVoice*,float raw1,float raw2,float gain1,float gain2,float *osc1,float *osc2);
VC_HELPER void vc_route_sample(VCSynth*,VCVoice*,float osc1,float osc2,float *left,float *right);
void vc_init(VCSynth*,float sr);
unsigned vc_param_max(unsigned id);
void vc_param(VCSynth*,unsigned id,unsigned value);
void vc_note_on(VCSynth*,int note,int velocity);
void vc_generated_note_off(VCSynth *,int note,unsigned release_limit);
void vc_generated_slide_on(VCSynth *,int note,int velocity,int previous);
void vc_note_off(VCSynth*,int note);
#ifdef VC_MATRIX
void vc_set_matrix(VCSynth*,const VCMatrix*);
void vc_note_off_velocity(VCSynth*,int note,int velocity);
void vc_pressure(VCSynth*,int value);
#endif
int vc_cc_param(int cc);
void vc_cc(VCSynth*,int cc,int value);
void vc_bend(VCSynth*,int value14);
void vc_panic(VCSynth*);
#ifdef VC_DUAL_RENDER
void vc_render_stereo(VCSynth*,float *left,float *right,unsigned frames);
#else
VC_HOT void vc_render_stereo(VCSynth*,float *left,float *right,unsigned frames);
#endif
void vc_render(VCSynth*,float *mono,unsigned frames);
void vc_profile_prepare(VCSynth*);
VC_HELPER void vc_profile_run(VCSynth*,unsigned mode,unsigned frames);
unsigned vc_active(const VCSynth*);
#ifdef VC_DUAL_RENDER
int vc_shed(VCSynth*);
#endif
#endif

void vc_lfo_set_extra_tables(const float *tables);

void vc_lfo_active_reset(void);
unsigned vc_lfo_active_shape(unsigned slot);
int vc_lfo_active_set(unsigned slot,unsigned shape,const float *table);

void vc_apply_patch(VCSynth *,const uint8_t *);

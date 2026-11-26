#include "../experimental/band_handoff.h"
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
static VCBandHandoff h;
static float tables[VC_BAND_SLOTS][2049],reference[64][14][2049],peaks[64];
static double coefficients[64][256],sines[2048];
static _Atomic unsigned done,audio_progress;
static unsigned background_calls;
static unsigned checks,boundaries;
static int load(void *ctx,unsigned wave,double out[256],float *peak){(void)ctx;memcpy(out,coefficients[wave],2048);*peak=peaks[wave];return 1;}
static void read_file(const char *p,void *data,size_t n){FILE *f=fopen(p,"rb");assert(f&&fread(data,n,1,f)==1);fclose(f);}
static void verify_audio(void){for(unsigned i=0;i<12;i++){unsigned key=h.audio_key[i];if(key==VC_BAND_OFF)assert(!h.audio_table[i]);else {assert(h.audio_table[i]);assert(!memcmp(h.audio_table[i],reference[key/14][key%14],8196));checks++;}}}
static void *worker(void *unused){(void)unused;for(unsigned i=0;i<30000||atomic_load_explicit(&audio_progress,memory_order_acquire)<1000;i++){vc_band_background(&h,4096);background_calls++;if(i%4==0)sched_yield();}atomic_store_explicit(&done,1,memory_order_release);return 0;}
int main(void){
 read_file("data/source-waves.f32",reference,sizeof(reference));read_file("data/wave-coefficients.f64",coefficients,sizeof(coefficients));read_file("data/wave-sines.f64",sines,sizeof(sines));read_file("experimental/wave-peaks.f32",peaks,sizeof(peaks));vc_band_handoff_init(&h,tables,sines,load,0);assert(atomic_is_lock_free(&h.published)&&atomic_is_lock_free(&h.desired[0]));
 /* Deterministic proof that a proposal cannot replace audio before a boundary. */
 vc_band_audio_request(&h,0,1,13);vc_band_background(&h,4096);assert(h.pending&&!h.audio_table[0]);assert(vc_band_audio_boundary(&h));verify_audio();vc_band_background(&h,0);const float *old=h.audio_table[0];vc_band_audio_request(&h,0,2,13);vc_band_background(&h,4096);assert(h.pending&&h.audio_table[0]==old);for(unsigned i=0;i<100;i++)vc_band_background(&h,4096);verify_audio();assert(h.audio_table[0]==old);vc_band_audio_boundary(&h);verify_audio();vc_band_background(&h,0);
 vc_band_audio_request(&h,0,3,13);
 pthread_t thread;atomic_init(&done,0);atomic_init(&audio_progress,0);assert(!pthread_create(&thread,0,worker,0));unsigned iteration=0;
 while(!atomic_load_explicit(&done,memory_order_acquire)){
  unsigned client=iteration%12;if(boundaries){if(iteration%37==0)vc_band_audio_release(&h,client);else vc_band_audio_request(&h,client,(iteration/12)%64,8+iteration%6);}
  boundaries+=vc_band_audio_boundary(&h);verify_audio();sched_yield();verify_audio();iteration++;if(boundaries)atomic_fetch_add_explicit(&audio_progress,1,memory_order_release);
 }
 assert(!pthread_join(thread,0));
 /* Stable final requests must converge; old pointers stay valid until commit. */
 for(unsigned i=0;i<12;i++)vc_band_audio_request(&h,i,40+i,13);
 unsigned ready=0;for(unsigned n=0;n<1000&&!ready;n++){vc_band_background(&h,4096);verify_audio();vc_band_audio_boundary(&h);verify_audio();ready=1;for(unsigned i=0;i<12;i++)ready&=h.audio_key[i]==(40+i)*14+13;}assert(ready);
 for(unsigned i=0;i<12;i++)vc_band_audio_release(&h,i);for(unsigned n=0;n<4;n++){vc_band_background(&h,4096);vc_band_audio_boundary(&h);}for(unsigned i=0;i<VC_BAND_SLOTS;i++)assert(!h.cache.slots[i].refs);verify_audio();
 assert(checks>100&&boundaries>0);
 printf("{\"table_checks\":%u,\"concurrent_boundary_swaps\":%u,\"audio_iterations\":%u,\"background_calls\":%u,\"delayed_ack_preserves_old_table\":true,\"final_requests_converge\":true,\"release_clears_pins\":true,\"host_atomics_lock_free\":true,\"host_handoff_bytes\":%zu,\"pass\":true}\n",checks,boundaries,iteration,background_calls,sizeof(h));
}

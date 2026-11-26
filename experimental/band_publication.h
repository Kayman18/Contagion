#ifndef VC_EXPERIMENTAL_BAND_PUBLICATION_H
#define VC_EXPERIMENTAL_BAND_PUBLICATION_H
#include <stdint.h>
/* FM1 option is limited to the current CPU0 audio ISR + CPU0 main-loop model.
 * Aligned 32-bit mailbox accesses and csync follow the BSP publication pattern.
 * No read-modify-write, locks, interrupt masking, or helper runtime required.
 * CPU1/multicore use is NOT certified by this adapter. */
#if defined(VC_BAND_CACHE) && !defined(PLAT_HOST) && !defined(VC_BAND_FM1_SINGLE_CORE)
#define VC_BAND_FM1_SINGLE_CORE 1
#endif
#ifdef VC_BAND_FM1_SINGLE_CORE
#ifdef PLAT_HOST
#error FM1 publication must not replace C11 atomics in host thread tests
#endif
typedef struct __attribute__((aligned(4))) {volatile uint32_t value;} VCBandWord;
_Static_assert(sizeof(unsigned)==4,"FM1 requires 32-bit unsigned");
_Static_assert(sizeof(VCBandWord)==4,"mailbox word must be 32-bit");
_Static_assert(_Alignof(VCBandWord)>=4,"mailbox must be aligned");
static inline __attribute__((always_inline)) void vc_band_word_init(VCBandWord *p,unsigned v){p->value=v;}
static inline unsigned vc_band_word_owned_value(const VCBandWord *p){return p->value;}
static inline __attribute__((always_inline)) unsigned vc_band_word_load(const VCBandWord *p){__asm__ volatile("csync" ::: "memory");unsigned v=p->value;__asm__ volatile("csync" ::: "memory");return v;}
static inline __attribute__((always_inline)) void vc_band_word_store(VCBandWord *p,unsigned v){__asm__ volatile("csync" ::: "memory");p->value=v;__asm__ volatile("csync" ::: "memory");}
#else
#include <stdatomic.h>
typedef _Atomic unsigned VCBandWord;
static inline void vc_band_word_init(VCBandWord *p,unsigned v){atomic_init(p,v);}
static inline unsigned vc_band_word_owned_value(const VCBandWord *p){return atomic_load_explicit(p,memory_order_relaxed);}
static inline unsigned vc_band_word_load(const VCBandWord *p){return atomic_load_explicit(p,memory_order_acquire);}
static inline void vc_band_word_store(VCBandWord *p,unsigned v){atomic_store_explicit(p,v,memory_order_release);}
#endif
/* Only the producer calls this. A matching value has already been published. */
static inline void vc_band_word_publish_changed(VCBandWord *p,unsigned v){
 if(vc_band_word_owned_value(p)!=v)vc_band_word_store(p,v);
}
#endif

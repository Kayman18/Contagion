/* Test-only: CPU1 as a real thread for the host simulator (build with -DDC_PTHREAD -pthread).
 * DC_JITTER=1 sometimes delays the worker's start, to stress the two-stage render. */
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
static pthread_t worker; static void (*job)(void*); static void *job_arg; static int running;
static void *run(void *unused){(void)unused;
 if(getenv("DC_JITTER")&&(rand()&3)==0){struct timespec t={0,(long)(rand()%300000)};nanosleep(&t,0);}
 job(job_arg);return 0;}
int dc_submit(void (*fn)(void*),void *arg){job=fn;job_arg=arg;running=pthread_create(&worker,0,run,0)==0;return running;}
int dc_wait(void){if(!running)return 1;pthread_join(worker,0);running=0;return 1;}
unsigned dc_status(void){return 2;}
unsigned dc_stage(void){return 3;}
unsigned dc_fault(void){return 0;}

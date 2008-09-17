#pragma JSEXT recursion 1
#define _PR_PTHREADS
#include <prlock.h>
#include <prcvar.h>
#include <prthread.h>
extern int pthread_detach (pthread_t __th) ;
#pragma JSEXT dl main

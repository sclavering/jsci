#ifndef __NSPR_H
#define __NSPR_H

#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define PR_BEGIN_EXTERN_C
#define PR_END_EXTERN_C
#define NSPR_API(x) x
#define PR_IMPLEMENT(x) x
#define PR_ASSERT(x)
#define PR_DELETE(x) free(x)
#define PR_NEWZAP(x) (x*)calloc(sizeof(x),1)
#define PR_NEW(x) (x*)malloc(sizeof(x))
#define _PT_PTHREAD_ZERO_THR_HANDLE(dest) ((dest)=0)
#define _PT_PTHREAD_COPY_THR_HANDLE(src, dest) ((dest)=(src))
#include "prtypes.h"

#endif

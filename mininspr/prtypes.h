#ifndef __PRTYPES_H
#define __PRTYPES_H

#include "nspr.h"

typedef int PRIntervalTime;
typedef int PRStatus;
typedef int PRInt32;
typedef int PRBool;
typedef unsigned int PRUint32;
typedef char PRInt8;
typedef int PRIntn;
typedef unsigned int PRUintn;
typedef unsigned int PRUptrdiff;
typedef void (*PRThreadPrivateDTOR) (void*);
#define PR_INTERVAL_NO_TIMEOUT 0
#define PR_FAILURE -1
#define PR_SUCCESS 0
#define PR_TRUE 1
#define PR_FALSE 0

#endif

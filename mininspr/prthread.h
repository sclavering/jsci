#ifndef _PRTHREAD_H
#define _PRTHREAD_H

int PR_GetCurrentThread();

#include "nspr.h"

PR_IMPLEMENT(PRStatus) PR_NewThreadPrivateIndex(PRUintn *newIndex, PRThreadPrivateDTOR dtor);
PR_IMPLEMENT(PRStatus) PR_SetThreadPrivate(PRUintn index, void *priv);
PR_IMPLEMENT(void*) PR_GetThreadPrivate(PRUintn index);

#endif

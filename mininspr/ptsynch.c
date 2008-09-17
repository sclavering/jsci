/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape Portable Runtime (NSPR).
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1998-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

/*
** File:            ptsynch.c
** Descritpion:        Implemenation for thread synchronization using pthreads
** Exports:            prlock.h, prcvar.h, prmon.h, prcmon.h
*/

#if defined(_PR_PTHREADS) || defined(_WIN32)

#include "primpl.h"
//...#include "obsolete/prsem.h"

#include <string.h>
#ifdef _WIN32
#else
# include <pthread.h>
# include <sys/time.h>
#endif

//...static pthread_mutexattr_t _pt_mattr;
//...static pthread_condattr_t _pt_cvar_attr;

#if defined(DEBUG)
extern PTDebug pt_debug;  /* this is shared between several modules */

#if defined(_PR_DCETHREADS)
static pthread_t pt_zero_tid;  /* a null pthread_t (pthread_t is a struct
                                * in DCE threads) to compare with */
#endif  /* defined(_PR_DCETHREADS) */
#endif  /* defined(DEBUG) */

/**************************************************************/
/**************************************************************/
/*****************************LOCKS****************************/
/**************************************************************/
/**************************************************************/

/*...*/

static void pt_PostNotifies(PRLock *lock, PRBool unlock)
{
    PRIntn index, rv;
    _PT_Notified post;
    _PT_Notified *notified, *prev = NULL;
    /*
     * Time to actually notify any conditions that were affected
     * while the lock was held. Get a copy of the list that's in
     * the lock structure and then zero the original. If it's
     * linked to other such structures, we own that storage.
     */
    post = lock->notified;  /* a safe copy; we own the lock */

#if defined(DEBUG)
    memset(&lock->notified, 0, sizeof(_PT_Notified));  /* reset */
#else
    lock->notified.length = 0;  /* these are really sufficient */
    lock->notified.link = NULL;
#endif

    /* should (may) we release lock before notifying? */
    if (unlock)
    {
#ifdef _WIN32
		rv = !ReleaseSemaphore(lock->sem, 1, NULL);
#else
        rv = pthread_mutex_unlock(&lock->mutex);
#endif
		PR_ASSERT(0 == rv);
    }

    notified = &post;  /* this is where we start */
    do
    {
        for (index = 0; index < notified->length; ++index)
        {
            PRCondVar *cv = notified->cv[index].cv;
            PR_ASSERT(NULL != cv);
            PR_ASSERT(0 != notified->cv[index].times);
            if (-1 == notified->cv[index].times)
            {
#ifdef _WIN32
				int waiting=cv->waiting;
				int wereWaiting=waiting;
                while (waiting)
                {
					WaitForSingleObject(cv->lock->sem, INFINITE); // acquire lock
					rv = !ReleaseSemaphore(cv->cv, 1, NULL);
					waiting--;
				}
				cv->waiting-=wereWaiting;
#else
                rv = pthread_cond_broadcast(&cv->cv);
#endif
				PR_ASSERT(0 == rv);
            }
            else
            {
                while (notified->cv[index].times-- > 0)
                {
#ifdef _WIN32
					WaitForSingleObject(cv->lock->sem, INFINITE);
					rv = !ReleaseSemaphore(cv->cv, 1, NULL);
					cv->waiting--;
#else
					rv = pthread_cond_signal(&cv->cv);
#endif
					PR_ASSERT(0 == rv);
                }
            }
#if defined(DEBUG)
            pt_debug.cvars_notified += 1;
            if (0 > PR_AtomicDecrement(&cv->notify_pending))
            {
                pt_debug.delayed_cv_deletes += 1;
                PR_DestroyCondVar(cv);
            }
#else  /* defined(DEBUG) */
            if (0 > PR_AtomicDecrement(&cv->notify_pending))
                PR_DestroyCondVar(cv);
#endif  /* defined(DEBUG) */
        }
        prev = notified;
        notified = notified->link;
        if (&post != prev) PR_DELETE(prev);
    } while (NULL != notified);
}  /* pt_PostNotifies */

PR_IMPLEMENT(PRLock*) PR_NewLock(void)
{
    PRIntn rv;
    PRLock *lock;

    //...    if (!_pr_initialized) _PR_ImplicitInitialization();

    lock = PR_NEWZAP(PRLock);
    if (lock != NULL)
    {
      //...        rv = _PT_PTHREAD_MUTEX_INIT(lock->mutex, _pt_mattr); 
#ifdef _WIN32
		rv = !(lock->sem=CreateSemaphore(NULL, 1, 1, NULL));
#else
        rv = pthread_mutex_init(&(lock->mutex), NULL);
#endif
        PR_ASSERT(0 == rv);
    }
#if defined(DEBUG)
    pt_debug.locks_created += 1;
#endif
    return lock;
}  /* PR_NewLock */

PR_IMPLEMENT(void) PR_DestroyLock(PRLock *lock)
{
    PRIntn rv;
    PR_ASSERT(NULL != lock);
    PR_ASSERT(_PT_PTHREAD_THR_HANDLE_IS_ZERO(lock->owner));
    PR_ASSERT(0 == lock->notified.length);
    PR_ASSERT(NULL == lock->notified.link);
#ifdef _WIN32
    rv = !CloseHandle(lock->sem);
#else
    rv = pthread_mutex_destroy(&lock->mutex);
#endif
	PR_ASSERT(0 == rv);
#if defined(DEBUG)
    memset(lock, 0xaf, sizeof(PRLock));
    pt_debug.locks_destroyed += 1;
#endif
    PR_DELETE(lock);
}  /* PR_DestroyLock */

PR_IMPLEMENT(void) PR_Lock(PRLock *lock)
{
    PRIntn rv;
    PR_ASSERT(lock != NULL);
#ifdef _WIN32
	rv = WaitForSingleObject(lock->sem, INFINITE);
#else
	rv = pthread_mutex_lock(&lock->mutex);
#endif
	PR_ASSERT(0 == rv);
    PR_ASSERT(0 == lock->notified.length);
    PR_ASSERT(NULL == lock->notified.link);
    PR_ASSERT(_PT_PTHREAD_THR_HANDLE_IS_ZERO(lock->owner));
#ifdef _WIN32
	lock->owner=GetCurrentThreadId();
#else
    _PT_PTHREAD_COPY_THR_HANDLE(pthread_self(), lock->owner);
#endif
#if defined(DEBUG)
    pt_debug.locks_acquired += 1;
#endif
}  /* PR_Lock */

PR_IMPLEMENT(PRStatus) PR_Unlock(PRLock *lock)
{
    PRIntn rv;

    PR_ASSERT(lock != NULL);
    PR_ASSERT(_PT_PTHREAD_MUTEX_IS_LOCKED(lock->mutex));
    PR_ASSERT(pthread_equal(lock->owner, pthread_self()));

#ifdef _WIN32
	if (lock->owner!=GetCurrentThreadId())
		return PR_FAILURE;
	lock->owner=0;
#else
    if (!pthread_equal(lock->owner, pthread_self()))
        return PR_FAILURE;
    _PT_PTHREAD_ZERO_THR_HANDLE(lock->owner);
#endif

    if (0 == lock->notified.length)  /* shortcut */
    {
#ifdef _WIN32
		rv = !ReleaseSemaphore(lock->sem, 1, NULL);
#else
        rv = pthread_mutex_unlock(&lock->mutex);
#endif
        PR_ASSERT(0 == rv);
    }
    else pt_PostNotifies(lock, PR_TRUE);

#if defined(DEBUG)
    pt_debug.locks_released += 1;
#endif
    return PR_SUCCESS;
}  /* PR_Unlock */


/**************************************************************/
/**************************************************************/
/***************************CONDITIONS*************************/
/**************************************************************/
/**************************************************************/


/*...*/


/*
 * Notifies just get posted to the protecting mutex. The
 * actual notification is done when the lock is released so that
 * MP systems don't contend for a lock that they can't have.
 */
static void pt_PostNotifyToCvar(PRCondVar *cvar, PRBool broadcast)
{
    PRIntn index = 0;
    _PT_Notified *notified = &cvar->lock->notified;

    PR_ASSERT(pthread_equal(cvar->lock->owner, pthread_self()));
    PR_ASSERT(_PT_PTHREAD_MUTEX_IS_LOCKED(cvar->lock->mutex));

    while (1)
    {
        for (index = 0; index < notified->length; ++index)
        {
            if (notified->cv[index].cv == cvar)
            {
                if (broadcast)
                    notified->cv[index].times = -1;
                else if (-1 != notified->cv[index].times)
                    notified->cv[index].times += 1;
                goto finished;  /* we're finished */
            }
        }
        /* if not full, enter new CV in this array */
        if (notified->length < PT_CV_NOTIFIED_LENGTH) break;

        /* if there's no link, create an empty array and link it */
        if (NULL == notified->link)
            notified->link = PR_NEWZAP(_PT_Notified);
        notified = notified->link;
    }

    /* A brand new entry in the array */
    (void)PR_AtomicIncrement(&cvar->notify_pending);
    notified->cv[index].times = (broadcast) ? -1 : 1;
    notified->cv[index].cv = cvar;
    notified->length += 1;

finished:
    PR_ASSERT(pthread_equal(cvar->lock->owner, pthread_self()));
}  /* pt_PostNotifyToCvar */

PR_IMPLEMENT(PRCondVar*) PR_NewCondVar(PRLock *lock)
{
    PRCondVar *cv = PR_NEW(PRCondVar);
    PR_ASSERT(lock != NULL);
    if (cv != NULL)
    {
      //...        int rv = _PT_PTHREAD_COND_INIT(cv->cv, _pt_cvar_attr); 
#ifdef _WIN32
		int rv = !(cv->cv=CreateSemaphore(NULL, 1, MAXLONG, NULL));
		cv->waiting = 0;
#else
        int rv = pthread_cond_init(&cv->cv, NULL);
#endif
		PR_ASSERT(0 == rv);
        cv->lock = lock;
        cv->notify_pending = 0;
#if defined(DEBUG)
        pt_debug.cvars_created += 1;
#endif
    }
    return cv;
}  /* PR_NewCondVar */

PR_IMPLEMENT(void) PR_DestroyCondVar(PRCondVar *cvar)
{
    if (0 > PR_AtomicDecrement(&cvar->notify_pending))
    {
#ifdef _WIN32
        PRIntn rv = !CloseHandle(cvar->cv);
		PR_ASSERT(0 == rv);
#else
        PRIntn rv = pthread_cond_destroy(&cvar->cv);
		PR_ASSERT(0 == rv);
#endif
#if defined(DEBUG)
        memset(cvar, 0xaf, sizeof(PRCondVar));
        pt_debug.cvars_destroyed += 1;
#endif
        PR_DELETE(cvar);
    }
}  /* PR_DestroyCondVar */

PR_IMPLEMENT(PRStatus) PR_WaitCondVar(PRCondVar *cvar, PRIntervalTime timeout)
{
    PRIntn rv;
    //...    PRThread *thred = PR_CurrentThread();

    PR_ASSERT(cvar != NULL);
    /* We'd better be locked */
    PR_ASSERT(_PT_PTHREAD_MUTEX_IS_LOCKED(cvar->lock->mutex));
    /* and it better be by us */
    PR_ASSERT(pthread_equal(cvar->lock->owner, pthread_self()));

    //...    if (_PT_THREAD_INTERRUPTED(thred)) goto aborted;

    /*
     * The thread waiting is used for PR_Interrupt
     */
    //...    thred->waiting = cvar;  /* this is where we're waiting */

    /*
     * If we have pending notifies, post them now.
     *
     * This is not optimal. We're going to post these notifies
     * while we're holding the lock. That means on MP systems
     * that they are going to collide for the lock that we will
     * hold until we actually wait.
     */
    if (0 != cvar->lock->notified.length)
        pt_PostNotifies(cvar->lock, PR_FALSE);

    /*
     * We're surrendering the lock, so clear out the owner field.
     */
#ifdef _WIN32
	cvar->lock->owner=0;
	cvar->waiting++;
    if (timeout == PR_INTERVAL_NO_TIMEOUT)
        rv = SignalObjectAndWait(cvar->lock->sem, cvar->cv, INFINITE, FALSE);
    else
      //...rv = pt_TimedWait(&cvar->cv, &cvar->lock->mutex, timeout);
      return PR_FAILURE;

    /* We just got the lock back - this better be empty */
    PR_ASSERT(_PT_PTHREAD_THR_HANDLE_IS_ZERO(cvar->lock->owner));
    cvar->lock->owner=GetCurrentThreadId();

#else
	_PT_PTHREAD_ZERO_THR_HANDLE(cvar->lock->owner);
    if (timeout == PR_INTERVAL_NO_TIMEOUT)
        rv = pthread_cond_wait(&cvar->cv, &cvar->lock->mutex);
    else
      //...rv = pt_TimedWait(&cvar->cv, &cvar->lock->mutex, timeout);
      return PR_FAILURE;

    /* We just got the lock back - this better be empty */
    PR_ASSERT(_PT_PTHREAD_THR_HANDLE_IS_ZERO(cvar->lock->owner));
    _PT_PTHREAD_COPY_THR_HANDLE(pthread_self(), cvar->lock->owner);

#endif


    PR_ASSERT(0 == cvar->lock->notified.length);
    //...    thred->waiting = NULL;  /* and now we're not */
    //...    if (_PT_THREAD_INTERRUPTED(thred)) goto aborted;
    if (rv != 0)
    {
      //...        _PR_MD_MAP_DEFAULT_ERROR(rv);
        return PR_FAILURE;
    }
    return PR_SUCCESS;

aborted:
    //...    PR_SetError(PR_PENDING_INTERRUPT_ERROR, 0);
    //...    thred->state &= ~PT_THREAD_ABORTED;
    return PR_FAILURE;
}  /* PR_WaitCondVar */

PR_IMPLEMENT(PRStatus) PR_NotifyCondVar(PRCondVar *cvar)
{
    PR_ASSERT(cvar != NULL);   
    pt_PostNotifyToCvar(cvar, PR_FALSE);
    return PR_SUCCESS;
}  /* PR_NotifyCondVar */

PR_IMPLEMENT(PRStatus) PR_NotifyAllCondVar(PRCondVar *cvar)
{
    PR_ASSERT(cvar != NULL);
    pt_PostNotifyToCvar(cvar, PR_TRUE);
    return PR_SUCCESS;
}  /* PR_NotifyAllCondVar */

/**************************************************************/
/**************************************************************/
/***************************MONITORS***************************/
/**************************************************************/
/**************************************************************/

/*...*/

/**************************************************************/
/**************************************************************/
/**************************SEMAPHORES**************************/
/**************************************************************/
/**************************************************************/

/*...*/

/**************************************************************/
/**************************************************************/
/******************ROUTINES FOR DCE EMULATION******************/
/**************************************************************/
/**************************************************************/

/*...*/

#endif  /* defined(_PR_PTHREADS) */

/* ptsynch.c */

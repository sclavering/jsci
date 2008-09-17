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
**     PR Atomic operations
*/


#include "pratom.h"
#include "primpl.h"

#include <string.h>

/*
 * The following is a fallback implementation that emulates
 * atomic operations for platforms without atomic operations.
 * If a platform has atomic operations, it should define the
 * macro _PR_HAVE_ATOMIC_OPS, and the following will not be
 * compiled in.
 */

#if !defined(_PR_HAVE_ATOMIC_OPS)

#if defined(_PR_PTHREADS) && !defined(_PR_DCETHREADS)
/*
 * PR_AtomicDecrement() is used in NSPR's thread-specific data
 * destructor.  Because thread-specific data destructors may be
 * invoked after a PR_Cleanup() call, we need an implementation
 * of the atomic routines that doesn't need NSPR to be initialized.
 */

/*
 * We use a set of locks for all the emulated atomic operations.
 * By hashing on the address of the integer to be locked the
 * contention between multiple threads should be lessened.
 *
 * The number of atomic locks can be set by the environment variable
 * NSPR_ATOMIC_HASH_LOCKS
 */

/*
 * lock counts should be a power of 2
 */
#define DEFAULT_ATOMIC_LOCKS	16	/* should be in sync with the number of initializers
										below */
#define MAX_ATOMIC_LOCKS		(4 * 1024)

static pthread_mutex_t static_atomic_locks[DEFAULT_ATOMIC_LOCKS] = {
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER,
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };

#ifdef DEBUG
static PRInt32 static_hash_lock_counts[DEFAULT_ATOMIC_LOCKS];
static PRInt32 *hash_lock_counts = static_hash_lock_counts;
#endif

static PRUint32	num_atomic_locks = DEFAULT_ATOMIC_LOCKS;
static pthread_mutex_t *atomic_locks = static_atomic_locks;
static PRUint32 atomic_hash_mask = DEFAULT_ATOMIC_LOCKS - 1;

#define _PR_HASH_FOR_LOCK(ptr) 							\
			((PRUint32) (((PRUptrdiff) (ptr) >> 2)	^	\
						((PRUptrdiff) (ptr) >> 8)) &	\
						atomic_hash_mask)

void _PR_MD_INIT_ATOMIC() {
  /*...*/
}

PRInt32
_PR_MD_ATOMIC_INCREMENT(PRInt32 *val)
{
    PRInt32 rv;
    PRInt32 idx = _PR_HASH_FOR_LOCK(val);

    pthread_mutex_lock(&atomic_locks[idx]);
    rv = ++(*val);
#ifdef DEBUG
    hash_lock_counts[idx]++;
#endif
    pthread_mutex_unlock(&atomic_locks[idx]);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_ADD(PRInt32 *ptr, PRInt32 val)
{
    PRInt32 rv;
    PRInt32 idx = _PR_HASH_FOR_LOCK(ptr);

    pthread_mutex_lock(&atomic_locks[idx]);
    rv = ((*ptr) += val);
#ifdef DEBUG
    hash_lock_counts[idx]++;
#endif
    pthread_mutex_unlock(&atomic_locks[idx]);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_DECREMENT(PRInt32 *val)
{
    PRInt32 rv;
    PRInt32 idx = _PR_HASH_FOR_LOCK(val);

    pthread_mutex_lock(&atomic_locks[idx]);
    rv = --(*val);
#ifdef DEBUG
    hash_lock_counts[idx]++;
#endif
    pthread_mutex_unlock(&atomic_locks[idx]);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_SET(PRInt32 *val, PRInt32 newval)
{
    PRInt32 rv;
    PRInt32 idx = _PR_HASH_FOR_LOCK(val);

    pthread_mutex_lock(&atomic_locks[idx]);
    rv = *val;
    *val = newval;
#ifdef DEBUG
    hash_lock_counts[idx]++;
#endif
    pthread_mutex_unlock(&atomic_locks[idx]);
    return rv;
}

#elif defined(_WIN32)

void _PR_MD_INIT_ATOMIC() {
  /*...*/
}

static __inline PRInt32
_PR_MD_ATOMIC_INCREMENT(PRInt32 *val)
{
	return InterlockedIncrement(val);
}

static __inline PRInt32
_PR_MD_ATOMIC_ADD(PRInt32 *ptr, PRInt32 val)
{
	return InterlockedExchangeAdd(ptr, val)+val;
}

static __inline PRInt32
_PR_MD_ATOMIC_DECREMENT(PRInt32 *val)
{
	return InterlockedDecrement(val);
}

static __inline PRInt32
_PR_MD_ATOMIC_SET(PRInt32 *val, PRInt32 newval)
{
	return InterlockedExchange(val, newval);
}

#else  /* _PR_PTHREADS && !_PR_DCETHREADS */


/*
 * We use a single lock for all the emulated atomic operations.
 * The lock contention should be acceptable.
 */
static PRLock *atomic_lock = NULL;
void _PR_MD_INIT_ATOMIC(void)
{
    if (atomic_lock == NULL) {
        atomic_lock = PR_NewLock();
    }
}

static _pr_initialized=0;
void _PR_ImplicitInitialization(void){
  _PR_MD_INIT_ATOMIC();
  _pr_initialized=1;
}

PRInt32
_PR_MD_ATOMIC_INCREMENT(PRInt32 *val)
{
    PRInt32 rv;

    if (!_pr_initialized) {
        _PR_ImplicitInitialization();
    }
    PR_Lock(atomic_lock);
    rv = ++(*val);
    PR_Unlock(atomic_lock);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_ADD(PRInt32 *ptr, PRInt32 val)
{
    PRInt32 rv;

    if (!_pr_initialized) {
        _PR_ImplicitInitialization();
    }
    PR_Lock(atomic_lock);
    rv = ((*ptr) += val);
    PR_Unlock(atomic_lock);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_DECREMENT(PRInt32 *val)
{
    PRInt32 rv;

    if (!_pr_initialized) {
        _PR_ImplicitInitialization();
    }
    PR_Lock(atomic_lock);
    rv = --(*val);
    PR_Unlock(atomic_lock);
    return rv;
}

PRInt32
_PR_MD_ATOMIC_SET(PRInt32 *val, PRInt32 newval)
{
    PRInt32 rv;

    if (!_pr_initialized) {
        _PR_ImplicitInitialization();
    }
    PR_Lock(atomic_lock);
    rv = *val;
    *val = newval;
    PR_Unlock(atomic_lock);
    return rv;
}
#endif  /* _PR_PTHREADS && !_PR_DCETHREADS */

#endif  /* !_PR_HAVE_ATOMIC_OPS */

void _PR_InitAtomic(void)
{
    _PR_MD_INIT_ATOMIC();
}

PR_IMPLEMENT(PRInt32)
PR_AtomicIncrement(PRInt32 *val)
{
    return _PR_MD_ATOMIC_INCREMENT(val);
}

PR_IMPLEMENT(PRInt32)
PR_AtomicDecrement(PRInt32 *val)
{
    return _PR_MD_ATOMIC_DECREMENT(val);
}

PR_IMPLEMENT(PRInt32)
PR_AtomicSet(PRInt32 *val, PRInt32 newval)
{
    return _PR_MD_ATOMIC_SET(val, newval);
}

PR_IMPLEMENT(PRInt32)
PR_AtomicAdd(PRInt32 *ptr, PRInt32 val)
{
    return _PR_MD_ATOMIC_ADD(ptr, val);
}


/*...*/

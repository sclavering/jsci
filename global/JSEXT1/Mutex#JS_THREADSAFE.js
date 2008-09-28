
(function(){

/*
      var m=new Mutex;

      m.lock();
      m.unlock();

  Used for synchronization. Only one thread
  can lock a mutex at a time. If several threads try
  to lock it at the same time, one of them (the last one)
  will wait until the first one unlocks it.

 */

  var m=function() {
    
    var obj;

    if (this.constructor != arguments.callee) {
      obj={};
      obj.__proto__=arguments.callee.prototype;
    } else
      obj=this;

    obj.mutex=mininspr.PR_NewLock();
    obj.mutex.finalize=mininspr.PR_DestroyLock;

    return obj;
  }
  /*
        m.lock()

    Locks the mutex. If the mutex is already locked
    by another thread, the call will block until it is unlocked.

   */

  m.prototype={
  lock: function() {
      mininspr.PR_Lock(this.mutex);
    },

  /*
        m.unlock()

    Unlocks the mutex.

   */
  unlock:function() {
      mininspr.PR_Unlock(this.mutex);
    }
  }

  return m;

})()

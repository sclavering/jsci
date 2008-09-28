/*

    new CondVar()

Use this if waiting for a condition:

    var cv=new CondVar();
    cv.wait (function(){return a>5});

At the same time, in another thread:

    cv.update (function(){a++;})

The first thread is awakened when _a_ becomes greater than 5.
Anything that might cause the condition to become true
should be executed in a function which is passed to _update_.
The test function is called every time update is called.
Several threads may be waiting on the same CondVar.

    */

(function(){

  var m=function() {
    var mutex=new Mutex();
    this.condVar=mininspr.PR_NewCondVar(mutex.mutex);
    this.condVar.finalize=mininspr.PR_DestroyCondVar;
    this.mutex=mutex;
  }

  /*
        cv.wait (func)

    Returns when _func_ returns _true_.
    _func_ is called immediately after _cv.wait_ is
    called and then each time _cv.update_ is called
    from another thread.
   */


  m.prototype.wait=function(func) {
    this.mutex.lock();
    while (!func()) {
      mininspr.PR_WaitCondVar(this.condVar, 0);
    }
    this.mutex.unlock();
  }

  /*
        cv.update (func)

    Calls _func_, which may update a variable
    which is being waited for. Do not alter such
    variables in any other way.
   */

  m.prototype.update=function(func) {
    this.mutex.lock();
    var ret=func();
    mininspr.PR_NotifyAllCondVar(this.condVar);
    this.mutex.unlock();
    return ret;
  }

  return m;

})()

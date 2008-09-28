(function(){

/*
      new Thread(func, [arg1,] [arg2,] ...)

  Starts a new thread which calls _func_ and ends when
  _func_ returns. Arguments no. 2 .. n are passed as arguments
  no. 1 .. n-1 to _func_. The new thread object is used
  as _func_'s _this_ object.
  
 */


  var t=function() {

    var obj;

    if (this.constructor != arguments.callee) {
      obj={};
      obj.__proto__=arguments.callee.prototype;
    } else
      obj=this;

    var pthread=arguments.callee.pthread;
    var pthread_create=arguments.callee.pthread_create;
    obj.ptr=new Pointer(pthread.pthread_t);
    Array.prototype.unshift.call(arguments,arguments.callee.startfunc)
    Array.prototype.unshift.call(arguments,obj.ptr)
    Array.prototype.unshift.call(arguments,obj)
    obj.joinlock=new Mutex;
    obj.parent=arguments.callee.threads[arguments.callee.getId()];
    pthread_create.apply(obj, arguments);
    obj.ptr.finalize=pthread.pthread_free;

    return obj;
  }

  /*
        obj = getCurrent()

    Returns the current thread object.
   */

  t.getCurrent=function() {
    var r=t.threads[t.getId()];
    if (!r)
      return t.threads[t.getId()]={};
    else
      return r;
  }

  /*
        num = getId()

    Returns the ID of the current thread
   */

  t.getId=function() {
    return mininspr.PR_GetCurrentThread();
  }

  t.threads={};
  t.global={};
  t.localcount={};
  t.threads[t.getId()]={local:t.global};

  /*
        unshare(propname)

    Makes propname a thread-local property of the global object.

   */

  t.unshare=function(propname) {

    if (t.localcount[propname]) {
      t.localcount[propname]++;
      return;
    }
  
    t.localcount[propname]=1;

    var obj=(function(){return this;})();

    t.global[propname]=obj[propname];

    delete obj[propname];
    obj.__defineGetter__(propname, threadGetter);
    obj.__defineSetter__(propname, threadSetter);

    function threadGetter() {
      return t.threads[t.getId()].local[propname];
    }

    function threadSetter(value) {
      t.threads[t.getId()].local[propname]=value;
    }
    
  }

  /*
        share(propname)

    Makes propname a shared property of the global object for all threads again.
    share / ushare keeps track of the number of calls to share/unshare per
    property and only makes a property global when share has been called as many times
    as unshare.
   */

  t.share=function(propname) {
    if (--t.localcount[propname])
      return;

    var obj=(function(){return this;})();

    delete obj[propname];
    obj[propname]=t.global[propname];
    delete t.global[propname];
  }

  /**/

  t.startfunc=function() {
    this.id=JSEXT1.Thread.getId();
    this.local={};
    this.local.__proto__=this.parent.local;
    JSEXT1.Thread.threads[this.id]=this;
    if (clib.unshare)
      clib.unshare(clib.CLONE_FS);
    var func=Array.prototype.shift.call(arguments);
    func.apply(this, arguments);
    delete JSEXT1.Thread.threads[this.id];
  }


  t.prototype={
    /**/
    lost: false,

  /*

        thr.join()

    Waits until the thread finishes.

   */

    join: function() {
    this.joinlock.lock();
    if (this.lost) {
      this.joinlock.unlock();
      throw new Error("thread detached or already joined");
    }
    this.lost=true;
    this.joinlock.unlock();
    var ret=[null];
    this.ptr.finalize=undefined;
    if (Thread.pthread.pthread_join(this.ptr.$, ret))
      throw new Error("pthread_join");
    },

  /*
        thr.detach()

    Detaches the thread, which means that its resources are
    freed when the thread finishes, without joining. Otherwise, thread resources
    are freed when the Thread object is garbage collected.

   */
  
    detach: function() {
    this.joinlock.lock();
    if (this.lost) {
      this.joinlock.unlock();
      throw new Error("thread already detached or joined");
    }
    this.lost=true;
    this.joinlock.unlock();

    this.ptr.finalize=undefined;
    if (mininspr.pthread_detach(this.ptr.$))
      throw new Error("pthread_detach");
  }
  }  
  return t;

})()

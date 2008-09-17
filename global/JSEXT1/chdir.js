  /*
        chdir(path)

    Change the current working directory to the specified path.
    Warning: On Linux < 2.6.16, changes path for all threads in a process.
    Use [[$curdir.chdirLock]] and [[$curdir.chdirUnlock]] instead when using threads.
  */

(function() {

  if (JSEXT_config.JS_THREADSAFE && !clib.unshare && Thread.threads.length)
    throw new Error("chdir can not be used in a multithreaded application on Linux<2.6.16. Try chdirLock / chdirUnlock instead");

  return function(dir) {
    if (clib.chdir(dir)==-1)
      throw new Error(os.error("chdir"));
  }

})()


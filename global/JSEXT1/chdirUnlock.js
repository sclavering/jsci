/*

      chdirUnlock()

  Change directory back to original working directory and
  release chdir lock.
  
 */

  function() {
    var od=chdirLock.olddir.pop();
    clib.fchdir(od);
    clib.close(od);
    if (chdirLock.cdlock && chdirLock.olddir.length==0) {
      delete chdirLock.cdthread;
      chdirLock.cdlock.unlock();
    }
  }

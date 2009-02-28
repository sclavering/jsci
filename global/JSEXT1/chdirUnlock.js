/*

      chdirUnlock()

  Change directory back to original working directory and
  release chdir lock.
  
 */

  function() {
    var od=chdirLock.olddir.pop();
    clib.fchdir(od);
    clib.close(od);
  }

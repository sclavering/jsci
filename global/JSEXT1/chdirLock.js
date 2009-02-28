/*
      chdirLock(string)

  Aquire a chdir lock and change the current working directory.
  Call [[$curdir.chdirUnlock]] after use, which will unlock the chdir lock
  and change directory back to the original working directory.

  This is necessary to use in a multi-thread application on
  Linux < 2.6.16 because cwd is shared between threads.
  
*/

(function() {
  function c(dir) {
    c.olddir.push(clib.open(".",clib.O_RDONLY));
    if (clib.chdir(dir)==-1) {
      clib.close(c.olddir.pop());
      throw new Error(os.error("chdirLock"));
    }
  }

  c.olddir=[];

  return c;
})()

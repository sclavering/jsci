  /*
          array = waitpid([pid] [, options])

      Wait for completion of a give child process.

      ### Return value ###

      An array containing [pid, status],
      where status is the exit code of the child process
      or a negative number if it was signaled.
     */

  function(pid, options) {
    if (pid==undefined) pid=-1;
    if (options==undefined) options=0;
    var status=[0];
    var res=clib.waitpid(pid,status,options);
    if (res==-1)
      throw new Error(os.error("waitpid"));
    if (status[0] & 0x7f) {
      return [res,-(status[0] & 0x7f)]; // signaled;
    } else {
      return [res,(status[0] & 0xff00) >> 8];
    }
  }

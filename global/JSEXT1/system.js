  /*

        num = system(cmd)

    Execute a shell command. Returns the return code of the
    process or a negative number if it was signaled
    or throws an error if it was not possible to start
    the child process.

        */

  function(cmd) {
    var ret;
    if (clib.system(cmd))
	throw new Error(os.error("system"));
    if (ret & 0x7f) {
      return -(ret & 0x7f); // signaled;
    } else {
      return (ret & 0xff00) >> 8;
    }
  }

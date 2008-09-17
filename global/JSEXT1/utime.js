  /*
          utime(path, [atime, [utime]]) -> None

      Set the access and modified time of the file to the given values.  If no times
      are given, set the access and modified times to the current time. Times should
      be Date objects.
     */

  function(path, atime, utime) {
    var now=new Date;
    if (arguments.length==1) {
      if (clib.utimes(path, null)) {
	throw new Error(os.error("utime"));
      }
    } else {
      if (atime===undefined) atime=now;
      if (utime===undefined) utime=now;
      var buf=[{tv_sec:atime.getTime()/1000}, {tv_sec:utime.getTime()/1000}];
      if (clib.utimes(path, buf)) {
	throw new Error(os.error("utime"));
      }
    }
  }

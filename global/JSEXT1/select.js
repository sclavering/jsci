  /*
        select(readers, writers, exceptions, timeout)

    The first arguments are arrays of file objects, the last is a number (milliseconds).
    All arguments are optional. No timeout value means _select_ will wait forever if needed.
    Availability: Unix
    
    ### Return value ##

    The passed arrays will be modified to only contain those
    files which are ready for action. Return value is the combined length of these arrays
    after modification.
  */

  function(readers, writers, exceptions, timeout) {
    var nfds=0;

    if (readers)
      var readfds=getfds(readers);
    else
      var readfds=null;

    if (writers)
      var writefds=getfds(writers);
    else
      var writefds=null;

    if (exceptions)
      var exceptfds=getfds(exceptions);
    else
      var exceptfds=null;

    if (timeout && timeout!=Infinity) {
      var tv=new Pointer(clib['struct timeval']);
      var sec=Math.floor(timeout/1000);
      tv.$={tv_sec: sec,
	    tv_usec: (timeout-sec*1000)*1000
           }
    } else {
      var tv=null;
    }
    
    var ret=clib.select(nfds+1, readfds, writefds, exceptfds, tv);
    if (ret==-1)
      throw new Error("select");

    if (readers)
      updatefds(readers, readfds);
    if (writers)
      updatefds(writers, writefds);
    if (exceptions)
      updatefds(exceptions, exceptfds);

    return ret;

    function updatefds(files, fds) {
      var i;
      for (i=0; i<files.length; i++)
	if (!clib.call_FD_ISSET(files[i].fileno(), fds)) {
	  files.splice(i,1);
	  i--;
	}
    }
    
    function getfds(files) {
      var i;
      var ret=new Pointer(clib.fd_set);
      clib.call_FD_ZERO(ret);
      for (var i=0; i<files.length; i++) {
	var fileno=files[i].fileno();
	clib.call_FD_SET(fileno, ret);
	if (fileno>nfds)
	  nfds=fileno;
      }
      return ret;
    }
  }

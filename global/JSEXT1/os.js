/*

  Contains a few os-related functions

 */

({
  /*
    string = os.strerror(errnum)

Returns error text for a given error number
     */

  strerror: function(errnum) {
    return clib.strerror(errnum).string();
  },

  /*
    string = os.error(string2)

Returns a string with the last error and
the description given in string.
*/

  error: function(str) {
    return str+": "+os.strerror(os.errno());
  },

  /*
        num = os.errno()

    Returns last error number
  */

  errno: function() {
    return clib.__errno_location().$;
  },

  /*
          string = os.confstr(num)

      Return a string-valued system configuration variable.
     */


  confstr: function(num) {
    var trylen=256;
    var buf=Pointer.malloc(trylen);
    for (;;) {
      var len=clib.confstr(num, buf, trylen);
      if (len<trylen) break;
      trylen*=2;
      buf.realloc(trylen);
    }
    return buf.string(len);
  },

  /*
          uname() -> (sysname, nodename, release, version, machine)

      Return an object identifying the current operating system.
     */

  uname: function() {
    var buf=Pointer(clib['struct utsname']);
    if (clib.uname(buf)) {
      throw new Error(os.error("uname"));
    }
    return {sysname: buf.member(0,"sysname").string(),
	    nodename: buf.member(0,"nodename").string(),
	    release: buf.member(0,"release").string(),
	    version: buf.member(0,"version").string(),
	    machine: buf.member(0,"machine").string()};
	    
  }

})

/*
Miscellaneous wrappers for os-related things in clib where the raw ffi interface is too horrible
*/
({
  /*
  string = getcwd()

  Return a string representing the path of the current working directory.
  */
  getcwd: function() {
    var trysize = 256;
    var buf = Pointer.malloc(trysize);
    for(;;) {
      if(clib.getcwd(buf, trysize) != null) break;
      trysize *= 2;
      buf.realloc(trysize);
    }
    return buf.string();
  },


  /*
  stat(path) -> obj
  stat(fileno) -> obj

  Perform a stat/fstat system call on the given path or integer file handle. Times are returned as date objects.
  */
  stat: function(path_or_fileno) {
    const buf = Pointer(clib['struct stat']);
    const func = typeof path_or_fileno == 'number' ? clib.call_fstat : clib.call_stat;
    if(func(path_or_fileno, buf) == -1) return null;

    var s = buf.$;
    var r = {};
    for(var rawprop in s) {
      var jsprop = rawprop.substr(3); // remove the "st_" prefix
      switch(rawprop) {
        case "st_atim":
        case "st_ctim":
        case "st_mtim":
          r[jsprop + "e"] = new Date(s[rawprop].tv_sec * 1000);
          break;
        case "st_dev":
        case "st_rdev":
          r[jsprop] = s[rawprop].$;
          break;
        default:
          r[jsprop] = s[rawprop];
      }
    }
    return r;
  },


  /*
  string = os.strerror(errnum)

  Returns error text for a given error number
  */
  strerror: function(errnum) {
    return clib.strerror(errnum).string();
  },


  /*
  string = os.error(string2)

  Returns a string with the last error and the description given in string.
  */
  error: function(str) {
    return str + ": " + os.strerror(os.errno());
  },


  /*
  num = os.errno()

  Returns last error number
  */
  errno: function() {
    return clib.__errno_location().$;
  },
})

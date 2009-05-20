/*
Miscellaneous wrappers for os-related things in clib where the raw ffi interface is too horrible
*/
({
  /*
  array = dir( [path="."] )

  Return a list containing the names of the entries in the directory, in abitrary order, and always excluding the special "." and ".." entries.
  */
  dir: function(path) {
    if(!path) path = ".";

    const d = clib.opendir(path);
    if(!d) throw new Error(os.error("dir " + path));

    const ret = [];
    for(;;) {
      var e = clib.readdir(d);
      if(e == null) break;
      e = e.$;
      var str = e.d_name;
      var len = str.indexOf("\0");
      if(len != -1) str = str.substr(0, len);
      if(str != "." && str != "..") ret.push(str);
    }
    clib.closedir(d);
    return ret;
  },


  /*
  bool = File.exists(path [, perms])

  Does the file exist, and can we open it with the supplied perms (e.g. "a", "w", etc., defaulting to "r").
  */
  exists: function(path, perms) {
    const f = clib.fopen(path, perms || "r");
    if(!f) return false;
    clib.fclose(f);
    return true;
  },


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
  isdir(path)

  Returns true if path exists and is a directory. Otherwise, returns false
  */
  isdir: function(path) {
    const ret = Pointer(clib['struct stat']);
    if(clib.call_stat(path, ret) == -1) return false;
    if((ret.field("st_mode").$ & clib.__S_IFMT) == clib.__S_IFDIR) return true;
    return false;
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

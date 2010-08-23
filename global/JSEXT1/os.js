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
  isdir(path)

  Returns true if path exists and is a directory. Otherwise, returns false
  */
  isdir: function(path) jsxlib.isDirOrLinkToDir(path),


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

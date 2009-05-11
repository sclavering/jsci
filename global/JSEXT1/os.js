/*
Miscellaneous wrappers for os-related things in clib where the raw ffi interface is too horrible
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

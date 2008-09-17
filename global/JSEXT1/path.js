  /*
          path(filename)
          
      Returns path part of the filename and path
     */

  function(filename) {
    var m=filename.match("(.*)"+JSEXT_config.sep+"([^"+JSEXT_config.sep+"]*)$");
    if (m) {
      if (m[1]!="")
        return m[1];
      else
	return JSEXT_config.sep;
    } else
      return JSEXT_config.curdir;
  }

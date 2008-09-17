/*
  
      bool = isrelative(path)

  Returns _true_ if _path_ is a relative path. False otherwise.

 */

  function(path) {
    return path[0]!=JSEXT_config.sep;
  }

  /*
          string = tmpnam([dir],[prefix])

      Return a unique name for a temporary file.
     */

  function(dir, prefix) {
    if (dir===undefined)
      dir=null;
    if (prefix===undefined)
      prefix=null;
  
    var ret=clib.tempnam(dir,prefix);
    if (ret==null)
      throw new Error(os.error("tmpnam"));
    var str=ret.string();
    clib.free(ret);
    return str;
  }

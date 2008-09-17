  /*
        isdir(path)

    Returns true if path exists and is a directory.
    Otherwise, returns false
  */

  function(path) {
    var ret=Pointer(clib['struct stat']);
    if (clib.call_stat(path,ret)==-1)
      return false;
    if ((ret.member(0,"st_mode").$ & clib.__S_IFMT) == clib.__S_IFDIR)
      return true;
    return false;
  }

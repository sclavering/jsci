  /*
        lstat(path) -> obj

    Like stat(path), but do not follow symbolic links.
  */

  function(path) {
    var ret=Pointer(clib['struct stat']);
    if (clib.call_lstat(path, ret)==-1) {
      return null;
    }
    return stat.unistat(ret);
  }

  /*
          path = readlink(path2)

      Return a string representing the path to which the symbolic link points.
     */

  function(path) {
    var trysize=256;
    var buf=Pointer.malloc(trysize);
    for(;;) {
      var len=clib.readlink(path, buf, trysize);
      if (len<trysize) break;
      trysize*=2;
      buf.realloc(trysize);
    }
    return buf.string(len);
  }


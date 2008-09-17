  /*
          string = getcwd()

      Return a string representing the current working directory.
  */

  function() {
    var trysize=256;
    var buf=Pointer.malloc(trysize);
    for(;;) {
      if (clib.getcwd(buf, trysize)!=null)
	break;
      trysize*=2;
      buf.realloc(trysize);
    }
    return buf.string();
  }

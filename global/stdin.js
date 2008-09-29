/*
     A File object representing the standard input.
    */

JSEXT_config._WIN32 ?
  new JSEXT1.File(clib.getstdin()) :
  new JSEXT1.File(clib.stdin.$)

/*
     A File object representing the standard error output.
    */

JSEXT_config._WIN32 ?
  new JSEXT1.File(clib.getstderr()) :
  new JSEXT1.File(clib.stderr.$)

/*
     A File object representing the standard output.
    */

JSEXT_config._WIN32 ?
  new JSEXT1.File(clib.getstdout()) :
  new JSEXT1.File(clib.stdout.$)

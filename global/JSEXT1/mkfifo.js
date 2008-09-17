  /*
          mkfifo(file, [mode=0666])

      Create a FIFO (a POSIX named pipe).
     */

  function(path, mode) {
    if (mode===undefined)
      mode=0666;
    if (clib.mkfifo(path,mode))
      throw new Error(os.error("mkfifo"));
  }

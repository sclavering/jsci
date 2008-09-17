  /*
          touch(path)

      Updates atime and mtime of path. Creates path if doesn't exist.

     */

  function(path) {
    if (!exists(path)) {
      var fd=clib.open(path, clib.O_CREAT | clib.O_RDWR, 0644);
      if (fd<0)
        throw new Error(os.error("touch"));
      clib.close(fd);
    } else
     clib.utime(path);
  }

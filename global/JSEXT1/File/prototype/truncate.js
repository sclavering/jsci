    /*

          file.truncate()

      Truncates the file at its current position, i.e. removes
      the rest of the file and reduces the file's size to the
      current file position.
    */
    
    function(size) {
      clib.ftruncate(this.fileno(),size);
    }


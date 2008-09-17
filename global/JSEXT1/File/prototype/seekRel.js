    /*
          file.seekRel (offset)
      
      Moves file pointer to the given position, relative to
      the current position.

      See also [[seek]] and [[seekEnd]].
    */

    function(offset) {
      clib.clearerr(this.fp);
      clib.fseek(this.fp, offset, clib.SEEK_CUR);
    }
    

    /*
          file.seek (offset)

      Moves file pointer to the given position.
      See also [[$curdir.seekEnd]] and [[$curdir.seekRel]].
    */

    function(offset) {
      clib.clearerr(this.fp);
      clib.fseek(this.fp, offset, clib.SEEK_SET);
    }
    

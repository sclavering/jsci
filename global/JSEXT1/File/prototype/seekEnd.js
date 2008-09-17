    /*
          file.seekEnd (offset)
      
      Moves file pointer to the given position, relative to
      the end of the file. _offset_ will generally be zero or a negative
      number.

      See also [[$curdir.seek]] and [[$curdir.seekRel]].
    */

    function(offset) {
      clib.clearerr(this.fp);
      clib.fseek(this.fp, offset, clib.SEEK_END);
    }
    

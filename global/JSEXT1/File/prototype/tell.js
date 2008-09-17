    /*
          num = file.tell()

      Returns the current file position.
    */
    
    function() {
      return clib.ftell(this.fp);
    }


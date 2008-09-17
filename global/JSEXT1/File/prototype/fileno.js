    /*

          num = file.fileno()

      Returns a number, the operating system's file number for the
      underlying file.
      
    */
    
    function() {
      return clib.fileno(this.fp);
    }


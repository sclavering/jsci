    /*

          bool = file.isatty()

      Returns true if the underlying file is a terminal.
      
    */

    function() {
      return clib.isatty(this.fileno());
    }
    

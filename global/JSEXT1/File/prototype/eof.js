    /*

          file.eof()

      Returns true if end-of-file has been reached.
      
    */
    
    function() {
	return (clib.feof(this.fp) || clib.ferror(this.fp))?true:false;
    }


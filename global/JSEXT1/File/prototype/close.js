    /*
          file.close()

      Close the file.
    */

    function() {
      if (!this.closed) {
	clib.fclose(this.fp);
	this.closed=true;
	this.fp.finalize=null;
      }
    }
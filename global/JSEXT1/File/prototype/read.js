    /*

          str = file.read(n)

      Reads _n_ bytes or until end of file, whichever comes first.
      Returns result as a string.

          str = file.read()

      Reads an entire file (from current position to end of file) and
      returns it as a string      
      
    */

    function(size) {
      size = Number(size || -1);
      if (size<0) {
	var trysize=4096;
	var ret="";
	while (!this.eof()) {
	  ret+=this.read(trysize);
	}
	return ret;
      }
      if (size==0)
        return "";
      var buf=Pointer.malloc(size);
      var len=clib.fread(buf, 1, size, this.fp);
      return buf.string(len);
    }
    

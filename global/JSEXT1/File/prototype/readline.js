    /*

          file.readline([size])

      Returns one line of text, including terminating newline character.
      If a _size_ is given, this will be the maximal line length.

      NOTE
      ----
      
      Not Binary-safe if operating system's fgets reads null characters
    */
    
    function(size) {
      if (arguments.length<1) size=-1;
      if (size<0) {
	var trysize=4096;
	var ret="";
	while (!this.eof()) {
	  ret+=this.readline(trysize);
	  if (ret[ret.length-1]=='\n') break;
	}
	return ret;
      }
      var buf=Pointer.malloc(size+1);
      //	var beforepos=this.tell();
      var line=clib.fgets(buf, size+1, this.fp);
      return buf.string();//(this.tell()-beforepos);
    }
    

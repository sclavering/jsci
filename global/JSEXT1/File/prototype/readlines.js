    /*

          array = file.readlines([size])

      Returns an array of lines with terminating newlines removed.
      If a _size_ argument is given, this is the maximal line length.
      
    */
    
    function(size) {
      if (arguments.length<1) size=-1;
      if (size<0) {
	var ret=[];
	while (!this.eof()) {
	  ret.push(this.readline());
	}
	return ret;
      }
      var buf=this.read(size);
      return buf.split(/\n/);
    }
    

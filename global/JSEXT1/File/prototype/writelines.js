    /*
          file.writelines (array)

      Writes an array of lines to file at the current position.
      Appends a newline character to each element of _array_.
    */
    
    function(obj) {
      for (var i=0; i<obj.length; i++) {
	this.write(obj[i]+"\n");
      }
    }

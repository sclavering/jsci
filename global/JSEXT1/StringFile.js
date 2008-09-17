(function(curdir){

    /*
          new StringFile( [string] )

      An object representing a string, with the
      same methods as [[$curdir.File]]. Can be passed
      to functions expecting a file.

     */

  function stringfile(str) {
    this.str=str || "";
    this.where=0;
    this.closed=false;
    this.name="<string>";
  }
  
  stringfile.prototype={
    closed: true,

    /*
          strfile.close()

      Close the stringfile.
    */

    close: function() {
      if (!this.closed) {
	delete this.string;
	this.closed=true;
      }
    },

    /*

          strfile.eof()

      Returns true if end-of-file has been reached.
      
    */
    
    eof: function() {
      return this.where>=this.str.length;
    },
  
    /*

          str = strfile.read(n)

      Reads _n_ bytes or until end of file, whichever comes first.
      Returns result as a string.

          str = strfile.read()

      Reads an entire file (from current position to end of file) and
      returns it as a string      
      
    */

    read: function(size) {
      if (arguments.length<1) size=-1;
      if (size<0) {
	var ret=this.str.substr(this.where);
	this.where=this.str.length;
      } else {
	var ret=this.str.substr(this.where,size);
	this.where+=size;
      }
      return ret;
    },
  
    /*

          strfile.readline()

      Returns one line of text, including terminating newline character.

      NOTE
      ----
      
      Binary-safe.
    */

    readline: function() {
      var nextpos=this.str.indexOf('\n',this.where);
      if (nextpos==-1) return this.read();
      
      var ret=this.str.substr(this.where, nextpos-this.where+1);
      this.where=nextpos+1;
      return ret;
    },
  
    /*

          array = strfile.readlines()

      Returns an array of lines with terminating newlines removed.
      
    */
    

    readlines: function() {
	return this.read().split("\n");
    },
  
    
    /*
          strfile.seek (offset)

      Moves file pointer to the given position.
      See also [[$curdir.seekEnd]] and [[$curdir.seekRel]].
    */

    seek: function(offset) {
	this.where=offset;
    },
  
    /*
          strfile.seekEnd (offset)
      
      Moves file pointer to the given position, relative to
      the end of the file. _offset_ will generally be zero or a negative
      number.

      See also [[$curdir.seek]] and [[$curdir.seekRel]].
    */

    seekEnd: function(offset) {
	this.where=this.str.length+offset;
    },
  
    /*
          strfile.seekRel (offset)
      
      Moves file pointer to the given position, relative to
      the current position.

      See also [[seek]] and [[seekEnd]].
    */

    seekRel: function(offset) {
	this.where+=offset;
    },
  
    /*
          num = strfile.tell()

      Returns the current file position.
    */

    tell: function() {
      return this.where;
    },
  
    /*

          strfile.truncate()

      Truncates the file at its current position, i.e. removes
      the rest of the file and reduces the file's size to the
      current file position.
    */

    truncate: function(size) {
      this.str=this.str.substr(0,this.where);
    },
  
    /*
          strfile.write (str)

      Writes _str_ to file at the current position.
      The argument is converted to a [[String]] before being
      written.
    */

    write: function(str) {
      str=String(str);
      if (this.eof()) {
	this.str+=str;
      } else {
	this.str=this.str.substr(0,this.where)+str+this.str.substr(this.where+str.length);
      }
      this.where+=str.length;
    },

    /*
          strfile.flush()

      Does nothing.
     */

    flush: function() {},
  
    /*
          strfile.writelines (array)

      Writes an array of lines to file at the current position.
      Appends a newline character to each element of _array_.
    */

    writelines: function(obj) {
      this.write(obj.join("\n")+"\n");
    }
  }

  return stringfile;

})(this)

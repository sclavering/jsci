
(function(curdir){

/*
  A file-like object which reads from an underlying
  file-like object containing chunked encoding.
  The chunk markers are consumed by _ChunkReader_
  and not passed on to the outside.

      stream = new ChunkReader( stream2 )

 */

  var c=function(file) {
    this.file=file;
  }

  c.prototype={

    iseof: false,
    closed: false,
    readbuf: "",

    /*

          stream.eof()

      Returns true if end-of-file has been reached.

     */

    eof: function() {
      return this.iseof;
    },

    readChunk: function() {

      var len=parseInt(this.file.readline(),16);

      if (len===0 || isNaN(len)) {
        this.iseof=true;
      } else {
        this.readbuf+=this.file.read(len);
	this.file.read(2); // CR LF
      }

    },

    /*

          str = stream.read(n)

      Reads _n_ bytes or until end of file, whichever comes first.
      Returns result as a string.

          str = stream.read()

      Reads an entire file (from current position to end of file) and
      returns it as a string      
      
    */

    read: function(len) {
      if (len===undefined) {
	while (!this.eof()) {
	  this.readChunk();
	}
	var ret=this.readbuf;
	this.readbuf="";
	return ret;
      }

      while (len>this.readbuf.length && !this.eof()) {
        this.readChunk();
      }

      var ret=this.readbuf.substr(0,len);
      this.readbuf=this.readbuf.substr(len);

      return ret;

    },

    /*

          stream.readline()

      Returns one line of text, including terminating newline character.

    */

    readline: function() {
      for (;;) {
	var nlpos=this.readbuf.indexOf('\n');
	if (nlpos!=-1 || this.eof())
	  break;
	this.readChunk();
      }

      if (nlpos==-1) {
	var ret=this.readbuf;
	this.readbuf="";
	return ret;
      }
      
      var ret=this.readbuf.substr(0,nlpos+1);
      this.readbuf=this.readbuf.substr(nlpos+1);
      return ret;
    },

    /*
          stream.close()

      Close the file.
      Reads to the end of the chunked data, but
      does not close the underlying file.

    */

    close: function() {
      if (this.closed)
        return;

      while (!this.eof()) {
	var len=parseInt(this.file.readline(),16);

	if (len===0 || isNaN(len)) {
	  this.iseof=true;
	} else {
	  this.file.read(len);
	}
      }

      this.closed=true;
    }

  }

  return c;

})(this)

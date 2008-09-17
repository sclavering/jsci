(function(curdir){

/*
  A file-like object which reads from an underlying
  file-like object containing content of a specified length.
  Reports end of file when the specified number of characters
  has been read.

      stream = new LengthReader( stream2, length )

 */

  var c=function(file, length) {
    this.file=file;
    this.remaining=Number(length);
  }

  c.prototype={

    closed: false,
    remaining: 0,

    /*

          stream.eof()

      Returns true if the specified number of characters has been read.

     */

    eof: function() {
      return this.remaining===0 || this.file.eof();
    },

    /*

          str = stream.read(n)

      Reads _n_ bytes or until the specified number of characters, whichever comes first.
      Returns result as a string.

          str = stream.read()

      Reads the specified number of characters from the
      underlying stream and
      returns them as a string      
      
    */

    read: function(len) {

      if (len===undefined) {
	len=this.remaining;
      }

      if (len>this.remaining)
        len=this.remaining;
      this.remaining-=len;
      if (len==0)
        return "";
      return this.file.read(len);

    },

    /*

          stream.readline()

      Returns one line of text, including terminating newline character.

    */

    readline: function() {

      var buf=this.file.readline();
      this.remaining-=buf.length;
      return buf;

    },

    /*
          stream.close()

      Close the file.
      Reads to the end of the specified number of characters, but
      does not close the underlying file.

    */

    close: function() {
      if (this.closed)
        return;

      if (this.remaining)
        this.file.read(this.remaining);

      this.closed=true;
    }

  }

  return c;

})(this)

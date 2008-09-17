(function(curdir){

/*
  A file-like object which reads from an underlying
  file-like object. Does not contain write functions.

      stream = new ReaderOnly( stream2 )

 */

  var c=function(file, length) {
    this.file=file;
  }

  c.prototype={

      /*
	Passed on to the underlying stream
      */

    eof: function() {
      return this.file.eof();
    },

      /*
	Passed on to the underlying stream
      */

    read: function(len) {
      return this.file.read(len);
    },

      /*
	Passed on to the underlying stream
      */

    readline: function(len) {
      return this.file.readline(len);
    },

      /*
	Passed on to the underlying stream
      */

    close: function() {
    }

  }

  return c;

})(this)

/*

    stream = new FlashWriter(stream2, headerFunc)

Create a flashwriter object and use it like a file.
Write, flush and close calls are forwarded to the
file passed to the constructor. Before the first
write, the _headerFunc_ is called, with _file_
as its only argument. If no write operations are
called, _headerFunc_ is called at _close_.

    */

(function(curdir){

  var c=function(file, headerFunc) {
    this.file=file;
    this.headerFunc=headerFunc;
  }

  c.prototype={

    /*
          stream.write (str)

      On the first call to _write_, _headerFunc_ (passed to the
      constructor) is called with one argument: The underlying
      stream. Then, _str_ is passed to the _write_ method of the 
      underlying stream.

      Subsequent calls to _write_ are passed to the underlying
      stream.

    */

    write: function(str) {
      if (this.headerFunc) {
	this.headerFunc(this.file);
	delete this.headerFunc;
      }
      return this.file.write(str);
    },

    /*
          stream.flush()

      Flushes the underlying stream.

     */

    flush: function() {
      return this.file.flush();
    },

    /*
          stream.close()

      If no write operations have
      been performed, the _headerFunc_ (passed to the constructor)
      is called with one argument: The underlying stream.
      Closes the underlying stream.

     */
    
    close: function() {
      if (this.headerFunc) {
	this.headerFunc(this.file);
	delete this.headerFunc;
      }
      
      this.file.close();
    }
    
  }

  return c;

})(this)

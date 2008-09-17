/*

    stream = new ChunkWriter(stream2)

Create a ChunkWriter object and use it like a file.
Write, flush and close calls are forwarded to the
file passed to the constructor. The data written
will be interlaced with hexadecimal length
specifiers in a chunked transfer encoding manner.
Chunks are only output on _flush_, _close_,
or when write operations accumulate more than 64KB
of data.

    */

(function(curdir){

  var c=function(file) {
    this.file=file;
  }

  c.prototype={

    closed: false,
    chunkbuf: "",

    /*
          stream.write (str)

      Writes _str_ to file at the current position.
      The argument is converted to a [[String]] before being
      written.

      ### Return value ###

      The length of _str_.
    */

    write: function(str) {
      str=String(str);
      this.chunkbuf+=str;
      if (this.chunkbuf.length>65536)
	this.flush();
      return str.length;
    },
    
    /*
      
          stream.flush()

      Flushes any data written so far and then flushes
      the underlying stream.

    */

    flush: function() {
      if (this.chunkbuf!="") {
	this.file.write(this.chunkbuf.length.toString(16)+"\r\n"+this.chunkbuf+"\r\n");
	this.chunkbuf="";
      }
      return this.file.flush();
    },

    /*
          stream.close()

      Flushes the stream and writes an chunk end marker to the
      underlying stream, but does not close it or flush it.
     */

    close: function() {
      if (this.closed)
        return;

      if (this.chunkbuf!="") {
	this.file.write(this.chunkbuf.length.toString(16)+"\r\n"+this.chunkbuf+"\r\n");
      }
      this.file.write("0\r\n");

      this.closed=true;
    }

  }

  return c;

})(this)

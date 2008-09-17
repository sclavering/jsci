(function(){

/*
    new File( FCGX_stream )

A wrapper class for the stdin, stdout and stderr streams.
Behaves like [[$parent.File]].

*/
  function file(stream) {
    this.stream=stream;
    this.closed=false;
  }
  
  file.prototype={
    closed: true,

    /*
          stream.close()
      
      Closes the stream
    */
    close:function() {
      if (!this.closed) {
	lib.FCGX_FClose(this.stream);
	this.closed=true;
      }
    },

    /*
      
    
        stream.eof()
    
    Returns true if end-of-file has been detected while reading
    from stream; otherwise returns false.
    
    ### Results ###

    true if end-of-file has been detected, false if not.
    
    
    */
    
    eof: function() {
      return lib.FCGX_HasSeenEOF(this.stream)?true:false;
    },
    
    /*
      
    
        stream.flush()
    
    Flushes any buffered output.
    
    Server-push is a legitimate application of flush.
    Otherwise, flush is not very useful, since accept
    does it implicitly.  Calling flush in non-push applications
    results in extra writes and therefore reduces performance.
    
    ### Results ###

    Throws an exception if an error occurred.
    
    
    */
    flush: function() {
      if (lib.FCGX_FFlush(this.stream)==-1)
        throw new Error("fcgi: flush");
    },

    /*
          stream.isatty()
      
      Returns false
    */

    isatty: function() {
      return false;
    },

    /*
      
    
        stream.read([n])
    
    Reads up to n consecutive bytes from the input stream
    Performs no interpretation of the input bytes.
    
    If no argument is given, reads until EOF is read
    
    ### Results ###

    A [[String]] containing the bytes read.  If the length is smaller than n,
    the end of input has been reached.
    
    
    */

    read: function(size) {
      if (arguments.length<1) size=-1;
      if (size<0) {
	var trysize=512;
	var ret="";
	while (!this.eof()) {
	  ret+=this.read(trysize);
	  trysize*=2;
	}
	return ret;
      }
      var buf=Pointer.malloc(Number(size));
      var len=lib.FCGX_GetStr(buf, size, this.stream);
      return buf.string(len);
    },


    // Not binary-safe
    /*
      
    
        stream.readline([n])
    
    Reads up to n consecutive bytes from the input stream
    into the character array str.  Stops before n bytes
    have been read if '\n' or EOF is read.  The terminating '\n'
    is copied to str.
    
    If no argument is given, reads until '\n' or EOF is read
    
    **Not binary-safe**: If the input stream contains \0 characters,
    they will cause reading to terminate.

    ### Results ###

    A [[String]] containing the bytes read.  If the length is smaller than n,
    the end of input has been reached.
    
    
    */

    readline: function(size) {
      if (arguments.length<1) size=-1;
      if (size<0) {
	var trysize=512;
	var ret="";
	while (!this.eof()) {
	  ret+=this.readline(trysize);
	  if (ret[ret.length-1]=='\n') break;
	  trysize*=2;
	}
	return ret;
      }
      var buf=Pointer.malloc(size+1);
      //      var beforepos=this.tell();
      var line=lib.FCGX_GetLine(buf, size+1, this.stream);
      if (!line) return "";
      return buf.string();
    },

    /*

        array = stream.readlines([size])
      
    Reads up to [size] bytes. If no argument is given, reads until
    EOF. Splits result into lines an returns an array of strings
    which do not contain the '\n' character.

    */

    readlines: function(size) {
      var buf=this.read(size);
      return buf.split(/\n/);
    },
    

    /*
      
    
        stream.write(str)
    
    Writes str.length consecutive bytes from the [[String]] str
    into the output stream.  Performs no interpretation
    of the output bytes.
    
    ### Results ###

    Number of bytes written (n) for normal return.
    Throws an exception if an error occurred.
    
    
    */

    write: function(str) {
      str=String(str);
      var ret=lib.FCGX_PutStr(str, str.length, this.stream);
      if (ret==-1)
        throw new Error("fcgi: write");
      return ret;
    },

    /*
      
        stream.writelines(array)
    
    Writes each of the strings in the array. Adds '\n'
    characters.
    */

    writelines: function(obj) {
      for (i in obj) {
	this.write(obj[i]+"\n");
      }
    }

  }

  return file;

})()
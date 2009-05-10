/*
fcgi([path] [, backlog])

Initializes the fcgi library and awaits incoming requests.
This function never returns. This function is
called by [[$curdir.shell]] when the environment variable
JSEXT_FCGI is set.

Only one request can be handled at once (since we don't use threads).

The .stdin, .stdout, .stderr and .environment properties of the global object are replaced with fcgi-specific versions.

### Arguments ###

* _path_: The file name of a named socket used for communication with the web server
* _backlog_: The maximum number of waiting requests

### NOTE ###

The JavaScript interpreter does not exit between requests, so any globally accessible objects that are created during the processing of one request are accessible to the next. In particular, modules that are loaded during the processing of one request remain loaded for the lifetime of the fcgi server, and are not automatically reloaded until the fcgi server is restarted. This is good for performance, but bad for development. Therefore, CGI is preferrable during development of a web site.
*/

(function() {

const libfcgi = JSEXT1.libfcgi;

function fcgi(path, backlog) {
  if(backlog === undefined) backlog = 8;

  if(0 != libfcgi.FCGX_IsCGI()) throw new Error("fcgi: isCgi");

  const fcgi_fd = path === undefined ? 0 : libfcgi.FCGX_OpenSocket(path, backlog);
  if(fcgi_fd == -1) throw new Error("fcgi:OpenSocket");

  try {
    while(true) {
      var req = new Request(fcgi_fd);

      stdin = req['in'];
      stderr = req['err'];
      stdout = req['out'];
      environment = req['env'];
      new CGI();

      req.close();
    }

  } catch(x) {
    // server shutdown?
  }

  if(fcgi_fd) clib.close(fcgi_fd);
}





/*
new File( FCGX_stream )

A wrapper class for the stdin, stdout and stderr streams.
Behaves like JSEXT1.File, in that they both suck.
*/
function File(stream) {
  this.stream = stream;
  this.closed = false;
}

File.prototype = {
  /*
  stream.close()
  */
  close:function() {
    if(this.closed) return
    libfcgi.FCGX_FClose(this.stream);
    this.closed = true;
  },

  /*
  stream.eof()
  
  Returns true if end-of-file has been detected while reading
  from stream; otherwise returns false.
  
  ### Results ###

  true if end-of-file has been detected, false if not.
  */
  eof: function() {
    return Boolean(libfcgi.FCGX_HasSeenEOF(this.stream));
  },

  /*
  stream.flush()
  
  Flushes any buffered output.  Should not be called explicitly, except for server-push.
  */
  flush: function() {
    if(libfcgi.FCGX_FFlush(this.stream) === -1) throw new Error("fcgi: flush");
  },

  /*
  stream.isatty()
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
    const buf = Pointer.malloc(Number(size));
    const len = libfcgi.FCGX_GetStr(buf, size, this.stream);
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
    const line = libfcgi.FCGX_GetLine(buf, size + 1, this.stream);
    if(!line) return "";
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
    const ret = libfcgi.FCGX_PutStr(str, str.length, this.stream);
    if(ret == -1) throw new Error("fcgi: write");
    return ret;
  },

  /*
  stream.writelines(array)
  
  Writes each of the strings in the array. Adds '\n'
  characters.
  */
  writelines: function(obj) {
    for (i in obj) this.write(obj[i]+"\n");
  },
}




/*
obj = new Request(fd)

Object which represents a request from an HTTP server to an
fcgi server.

Contains the following properties:

* _in_: A [[$curdir.File]] object representing the per-request standard input
* _out_: A [[$curdir.File]] object representing the per-request standard output
* _err_: A [[$curdir.File]] object representing the per-request standard error
* _env_: An object containing the per-request environment variables

### Arguments ###

* _fd_: The file descriptor used for communication with the HTTP server.

*/
function Request(fd) {
  this.request=new Pointer(libfcgi['struct FCGX_Request']);
  var res = libfcgi.FCGX_InitRequest(this.request, fd, 0);

  if(res != 0) throw new Error("FCGX_InitRequest");

  var res = libfcgi.FCGX_Accept_r(this.request);
  if(res != 0) throw new Error("FCGX_Accept_r");
  this.request.finalize=libfcgi.FCGX_Finish_r;

  this['in']=new File(this.request.member(0,"in").$);
  this['out']=new File(this.request.member(0,"out").$);
  this['err']=new File(this.request.member(0,"err").$);
  this['env']={};

  var envp=this.request.member(0,"envp").$;
  if (envp==null) return;
  var i;
  for(i=0;;i++) {
    var val=envp.member(i).$;
    if (val==null) break;
    val=val.string();
    var eqPos=val.indexOf("=");
    this.env[val.substr(0,eqPos)]=val.substr(eqPos+1);
  }
}

/*
  req.close()

Finish & free the request (multi-thread safe).

Side effect:
---

Finishes the request accepted by (and frees any
storage allocated by) the previous call to accept.


*/

Request.prototype.close=function() {
  this.request.finalize=null;
  libfcgi.FCGX_Finish_r(this.request);
}



return fcgi;

})()

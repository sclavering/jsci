/*
fcgi([path] [, backlog])

Initializes the fcgi library and awaits incoming requests.  Called on startup if the environment variable JSEXT_FCGI is set.  This function never returns.

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
    while(true) _fcgi_handle_request(fcgi_fd);
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
}




function _fcgi_handle_request(fd) {
  var rq = new Pointer(libfcgi['struct FCGX_Request']);

  var res = libfcgi.FCGX_InitRequest(rq, fd, 0);
  if(res != 0) throw new Error("FCGX_InitRequest");

  var res = libfcgi.FCGX_Accept_r(rq);
  if(res != 0) throw new Error("FCGX_Accept_r");
  rq.finalize = libfcgi.FCGX_Finish_r;

  stdin = new File(rq.field("in").$);
  stdout = new File(rq.field("out").$);
  stderr = new File(rq.field("err").$);
  environment = _fcgi_get_env(rq);

  new CGI();
  gc();
  libfcgi.FCGX_Finish_r(rq);
}

function _fcgi_get_env(rq) {
  var env = {};
  var envp = rq.field("envp").$;
  if (envp==null) return;
  var i;
  for(i=0;;i++) {
    var val = envp[i];
    if (val==null) break;
    val=val.string();
    var eqPos=val.indexOf("=");
    env[val.substr(0, eqPos)] = val.substr(eqPos + 1);
  }
  return env;
}



return fcgi;

})()

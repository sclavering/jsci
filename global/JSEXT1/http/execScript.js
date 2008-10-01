/*

     This function executes a JavaScript file on a web server.

         execScript([refresh=false])

     No parameters are passed to the function, but it must be
     passed a _this_ object which contains the following properties:

     * _hostdir_: An [[$parent.ActiveDirectory]] object representing the root directory
      where the file is stored.
     * _filename_: A string containing an absolute path, where root is
      the specified host directory, not the filesystem root.

     The script in _filename_ can access $parent.$parent... up to
     the level of _hostdir_, but not above it.

     Executes a script file, usually with a _.jsx_ extension.
     This is a three-step procedure:

     1. The script file is interpreted
     2. The script is executed.
     3. If the script evaluates to a function, i.e. if the last
        expression in the file is a function, that function will
        be called.

     Phases 1 and 2 are cached, so that each script
     file is only interpreted and executed once during the lifetime of the
     JavaScript session. However, the modification time is checked
     for each call to execScript, and if the script file is modified,
     it will be reinterpreted. Phase 3 is repeated
     for each call to execScript.

     In phase 2, the _this_ object, and the first object
     in the scope chain, is an ActiveDirectory object representing
     the directory in which the script file is stored. This means
     that a script can call functions in the same directory as
     itself without specifying any path.

     In phase 3, the _this_ object passed to the function
     is an object containing the following properties:

     * _requestHeaders_: An object containing request headers. The property names
       correspond to header field names, whose case is normalized
       to Capitalized-Hyphen-Case.
     * _responseHeaders_: An object containing response headers to be sent.
     * _method_: A string containing the HTTP method (usually "GET" or "POST").
     * _requestURL_: A string
     * _remoteAddress_: A string
     * _cookie_: A read/write property giving access to cookies. See separate
       article on [[Cookies]].

     If the Content-Type is neither "text/JSON", "application/x-www-form-urlencoded"
     or "multipart/form-data", the body of the request is not read by execScript.
     In that case, the script can read from [[stdin]] in phase 3.

     If the function takes no arguments, the body of the request is also not read by execScript.
     In that case, the script can read from [[stdin]] in phase 3.

     Any output to _stdout_ in phase 2 or 3 (usually using print)
     is sent to the client. The _responseHeaders_
     are automatically sent the first time anything is written to stdout.
     The default Content-Type is text/html. Note that the connection is
     buffered, so you may need to use stdout.flush() in order to actually
     send data over the network connection.

     Any exceptions produced during the execution of a script are
     caught and sent as HTML text to the client.

     execScript returns no value.

     ### Arguments ###

     * _refresh_: Boolean. If true, empties script cache.

     ### Return value ###
     
     This function does not return a value, but prints its
     output on [[stdout]].

    */

(function() {

var scriptCache={};

return function (refresh) {

  try {
    
    if (refresh)
      scriptCache={};
    
    var dir=this.hostdir;
    var filename=this.filename;
    
    this.responseHeaders.cacheControl="no-cache";
    
    var pathparts=filename.split(JSEXT_config.sep);
    var curdir=dir;
    for (var i=1; i<pathparts.length-1; i++) {
      curdir=curdir[pathparts[i]];
    }
    
    var onlyFilename=$parent.filename(filename);

    var script=scriptCache[curdir.$path+JSEXT_config.sep+onlyFilename];
    var mtime=$parent.stat(curdir.$path+JSEXT_config.sep+onlyFilename).mtime;
    
    if (!script || mtime>script.mtime) {
      var w;
      if (onlyFilename!="with.js" && onlyFilename!="with.jsx" && (w=curdir['with'])) {
	var before="";
	for (var i=w.length; i--;)
	  before+="with(this['with']["+i+"])";
	before+="with(this){";
	script=load.call(curdir,curdir.$path+JSEXT_config.sep+onlyFilename,before,"}");
      } else {
	script=load.call(curdir,curdir.$path+JSEXT_config.sep+onlyFilename);
      }
      script.mtime=mtime;
      scriptCache[curdir.$path+JSEXT_config.sep+onlyFilename]=script;
    }

    var func=script;

    if (typeof(func)==="function" && (func.name==="" || func.name==="anonymous")) {
      var cookies=this.requestCookies;
      delete this.requestCookies;

      this.GET_data = getGetData.call(this);
      this.POST_data = getPostData.call(this);
      this.cookie_data = cookies;
      func.call(this);
      setTimeout.exec();
    }
    
  } catch (x) {

    print(' <br/>\n');
    if (x.fileName && x.lineNumber)
      print('Line '+x.lineNumber+' in '+x.fileName+':');
    print(x+' <br/>\n');
    if (x.stack) {
      var stack=x.stack.split('\n');
      if (stack.length>6)
	print(stack.slice(1,stack.length-5).join(' <br/>\n')+' <br/>\n');
    }

  }
  
}


})()

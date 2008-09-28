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

     Parameters from the HTTP request are passed as arguments to the
     function. If
     the function takes an argument whose name corresponds to the name
     of an HTTP parameter, that parameter is passed as that argument. If
     there are HTTP parameters with a numerical name ("0", "1" etc.),
     those parameter are passed as the corresponding arguments to the
     function. All arguments will be strings unless they are JSON-encoded.
     JSON-encoding in HTTP GET and POST (url-encoded) can be done by providing a query string
     which consists of the innards of a JSON array (i.e. 1,"2",3) instead of
     the usual name/value-pairs.
     For HTTP POST multipart/form-data,
     it is possible to use Content-Type: text/JSON for
     some or all of the parameters to encode other types.

     The special argument names _$get_, _$post_ and _$cookie_ will receive
     an object containing all get, post and cookie values, respectively.

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

     If the function returns a string, that string is sent to the client.
     If the function returns another type of value, the Content-Type is set to text/JSON
     and the value is sent in the JSON format.

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

      if (!script.names) {
	var src=func.toSource();
	var names=src.match(/\(([^()]*)\)/)[1].split(", ");
	if (names[0]=="") names=[];
	script.names=names;
      }

      var cookies=this.requestCookies;
      delete this.requestCookies;

      this.GET_data = getGetData.call(this);
      this.POST_data = getPostData.call(this);
      this.cookie_data = cookies;
      var ret = func.call(this);

      switch(typeof(ret)) {

      case 'string':
	print(ret);
	break;

      case 'xml':
	this.responseHeaders.contentType='text/xml; charset="UTF-8"';
	print('<?xml version="1.0" encoding="UTF-8"?>\n');
	print($parent.encodeUTF8(String(ret)));
	break;

      case 'undefined':
	break;

      default:
	this.responseHeaders.contentType="text/JSON";
	print($parent.encodeJSON(ret));
	break;
      }

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

({
  /* cgi.run([refresh=false])

     Interprets the environment variables given in
     _environment_ according to the CGI/1.1 specification:
     <http://hoohoo.ncsa.uiuc.edu/cgi/env.html>.

     The CGI standard treats some HTTP headers differently
     from others. That distinction is reversed by the _run_
     function. All headers are placed in the
     requestHeaders property of the created context object.

     This function uses the web server's url-to-path translation.

     It should only be used for _.jsx_ files, which should contain a single
     anonymous javascript function.  The function will be called with no
     arguments, but with _this_ set to a context object with the following
     properties:

     ### Context object ###

     When _cgi_ calls a _.jsx_ function, it passes a _context object_
     as _this_, which contains the following properties:

     * _requestHeaders_: An object containing the following properties:
       * _contentType_
       * _contentLength_
       * all other HTTP headers passed from the HTTP server
     * _responseHeaders_: An object containing the following property:
       * _contentType_: "text/html"
     * _remoteAddress_
     * _method_: "GET" or "POST"
     * _requestURL_
     * _GET_data_: the decoded data from the query string
     * _POST_data_: the decoded data from a POST request, if any
     * _cookie_data_: the request cookies, as a name–value mapping

     ### Arguments ###

     * _refresh_: Boolean. If true, _cgi_ will attempt to reload
       ActiveDirectory objects that have been changed since they
       were last loaded.
  */
  run: function(refresh) {
    var sendcookies;

    stdout = new http.FlashWriter(stdout, function(stream) {
      stdin.close();
      if(cx.responseHeaders.location) cx.responseLine = "302 Found";
      if(cx.responseLine) stream.write("Status: " + cx.responseLine + "\r\n");
      sendcookies(stream);
      mime.writeHeaders(stream, cx.responseHeaders);
      delete cx.responseHeaders;
    });

    try {
      var cx = {};
      var headers = {};
      for(var i in environment) {
        if(i.substr(0,5) != "HTTP_") continue;
        var mangled = i.substr(5).toLowerCase().replace(/_./g, function(a) {
            return a.toUpperCase().substr(1);
          });
        headers[mangled] = environment[i];
      }

      if(environment.CONTENT_TYPE) headers.contentType = environment.CONTENT_TYPE;
      if(environment.CONTENT_LENGTH) headers.contentLength = environment.CONTENT_LENGTH;
      cx.requestHeaders = headers;
      cx.remoteAddress = environment.REMOTE_ADDR;
      cx.method = environment.REQUEST_METHOD;
      cx.requestURL = "http://" + (cx.requestHeaders.host || '') + environment.REQUEST_URI;
      // set in _execScript
      cx.POST_data = null;
      cx.GET_data = null
      cx.cookie_data = null;

      // Set a default content type.
      cx.responseHeaders = { contentType: 'text/html' };

      sendcookies = http.Cookie.bake.call(cx);

      var filename=environment.PATH_TRANSLATED || environment.SCRIPT_FILENAME;
      var path = $curdir.path(filename);
      var onlyFilename = $curdir.filename(filename);
      var urlpath = environment.PATH_INFO || environment.SCRIPT_NAME;
      var pathParts = path.split(JSEXT_config.sep);
      var urlParts = cx.requestURL.split("/");
      urlParts.pop();

      var i;
      for(i = 1; i <= urlParts.length; i++)
        if(pathParts[pathParts.length-i] != urlParts[urlParts.length-i])
          break;

      var root = pathParts.slice(0, pathParts.length - i + 1).join(JSEXT_config.sep);
      var rooturl = urlParts.slice(0, urlParts.length - i + 1).join("/");
      if(rooturl != "/") rooturl += "/";

      pathParts.push(onlyFilename);
      var relFilename = JSEXT_config.sep+pathParts.slice(pathParts.length - i).join(JSEXT_config.sep);

      var extension = relFilename.match(/\.([^.]*)$/);
      if(extension) extension = extension[1];

      if(refresh) {
        var global = (function() { return this; })();
        this._hostDirCache = {};
        this._clientDirCache = {};
        global.$checkdates();
        js['export'].global.$checkdates();
      }

      var hostdir = this._hostDirCache[rooturl];
      if(!hostdir) {
        hostdir = {};
        ActiveDirectory.call(hostdir, root);
        this._hostDirCache[rooturl] = hostdir;
      }

      var clientdir = this._clientDirCache[rooturl];
      if(!clientdir) {
        clientdir = {};
        ActiveDirectory.call(clientdir, root, js['export'].handlers, js['export'].platform);
        this._clientDirCache[rooturl] = clientdir;
        var relroot = $curdir.path(urlpath.replace(/\//g, JSEXT_config.sep));
        relroot = relroot.split(JSEXT_config.sep);
        relroot = relroot.slice(0, relroot.length - i + 1).join("/");
        if(relroot!="/") relroot += "/";
        clientdir.$url = relroot;
      }

      cx.clientdir = clientdir;
      cx.hostdir = hostdir;
      cx.filename = "/" + pathParts.slice(pathParts.length - i).join(JSEXT_config.sep);
      //  cx.responseLine="200 OK";

      this._execScript(cx, refresh);
    } catch(x) {
      print(' <br/>\n');
      if(x.fileName && x.lineNumber) print('Line ' + x.lineNumber + ' in ' + x.fileName + ':');
      print(x + ' <br/>\n');
      if(x.stack) {
        var stack = x.stack.split('\n');
        if(stack.length > 6) print(stack.slice(1, stack.length - 5).join(' <br/>\n') + ' <br/>\n');
      }
    }
    stdout.close();
  },
  _hostDirCache: {},
  _clientDirCache: {},


  /*
     This function executes a JavaScript file on a web server.

         execScript([refresh=false])

     No parameters are passed to the function, but it must be
     passed a _this_ object which contains the following properties:

     * _hostdir_: An [[ActiveDirectory]] object representing the root directory
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
  _execScript: function(cx, refresh) {
    if(refresh) this._execScript_cache = {};
    try {
      var dir = cx.hostdir;
      var filename = cx.filename;

      cx.responseHeaders.cacheControl = "no-cache";

      var pathparts = filename.split(JSEXT_config.sep);
      var curdir = dir;
      for(var i = 1; i < pathparts.length - 1; i++) curdir = curdir[pathparts[i]];

      var onlyFilename = JSEXT1.filename(filename);

      var script = this._execScript_cache[curdir.$path + JSEXT_config.sep + onlyFilename];
      var mtime = stat(curdir.$path + JSEXT_config.sep + onlyFilename).mtime;

      if(!script || mtime > script.mtime) {
        var w;
        if(onlyFilename != "with.js" && onlyFilename != "with.jsx" && (w = curdir['with'])) {
          var before="";
          for(var i = w.length; i--; ) before += "with(this['with'][" + i + "])";
          before += "with(this){";
          script = load.call(curdir, curdir.$path + JSEXT_config.sep + onlyFilename, before, "}");
        } else {
          script = load.call(curdir, curdir.$path+JSEXT_config.sep + onlyFilename);
        }
        script.mtime = mtime;
        this._execScript_cache[curdir.$path+JSEXT_config.sep + onlyFilename] = script;
      }

      var func = script;
      if(typeof(func) === "function" && (func.name === "" || func.name === "anonymous")) {
        var cookies = cx.requestCookies;
        delete cx.requestCookies;
        cx.GET_data = http.getGetData.call(cx);
        cx.POST_data = http.getPostData.call(cx);
        cx.cookie_data = cookies;
        func.call(cx);
        setTimeout.exec();
      }
    } catch(x) {
      print(' <br/>\n');
      if(x.fileName && x.lineNumber) print('Line ' + x.lineNumber + ' in ' + x.fileName + ':');
      print(x + ' <br/>\n');
      if(x.stack) {
        var stack = x.stack.split('\n');
        if(stack.length > 6) print(stack.slice(1, stack.length - 5).join(' <br/>\n') + ' <br/>\n');
      }
    }
  },
  _execScript_cache: {},
})

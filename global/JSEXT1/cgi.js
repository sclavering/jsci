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
      // these are currently set in execScript.js
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
      
      // 2008-09-28-22-26 steve: entirely removed the dispatching based on file extension.
      http.execScript.call(cx, refresh);
    
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
})

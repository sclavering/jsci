/*
new CGI([refresh = false])

Interprets the environment variables given in _environment_ according to the
CGI/1.1 specification: <http://hoohoo.ncsa.uiuc.edu/cgi/env.html>.

The .jsx file to be executed should contain a single anonymous function, which
will be called with the _CGI_ instance as its only argument.

### Arguments ###

* _refresh_: Boolean. If true, _cgi_ will attempt to reload ActiveDirectory
  objects that have been changed since they were last loaded.

### Properties ###

* _GET_data_: the decoded data from the query string
* _POST_data_: the decoded data from a POST request, if any

* _requestHeaders_: An object containing the following properties:
  * _contentType_
  * _contentLength_
  * all other HTTP headers passed from the HTTP server
* _responseHeaders_: An object containing the following property:
  * _contentType_: "text/html"
* _remoteAddress_: A string
* _method_: A string containing the HTTP method (usually "GET" or "POST").
* _requestURL_: A string
* _cookie_data_: the request cookies, as a name-value mapping
* _filename_: A string containing an absolute path, where root is the specified
  host directory, not the filesystem root.
* _hostdir_: An [[ActiveDirectory]] object for the root directory where the
  file is stored.

### Methods ###
*/

(function() {

function CGI(refresh) {
  this.requestHeaders = this._get_request_headers();

  this.remoteAddress = environment.REMOTE_ADDR;
  this.method = environment.REQUEST_METHOD;
  this.requestURL = "http://" + (this.requestHeaders.host || '') + environment.REQUEST_URI;

  this.GET_data = http.decodeURI(this.requestURL).qry || {};
  this.POST_data = this._get_POST_data();
  this.cookie_data = this._parse_cookie_header(this.requestHeaders.cookie);

  this.response_cookies = {};
  // Set a default content type.
  this.responseHeaders = { contentType: 'text/html' };

  this.run(refresh);
}

CGI.prototype = {
  run: function(refresh) {
    const cx = this;

    stdout = new http.FlashWriter(stdout, function(stream) {
      stdin.close();
      if(cx.responseHeaders.location) cx.responseLine = "302 Found";
      if(cx.responseLine) stream.write("Status: " + cx.responseLine + "\r\n");
      cx.output_cookies(stream);
      mime.writeHeaders(stream, cx.responseHeaders);
      delete cx.responseHeaders;
    });

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
        global.$checkdates();
        js['export'].global.$checkdates();
      }

      var hostdir = this._hostDirCache[rooturl];
      if(!hostdir) {
        hostdir = {};
        ActiveDirectory.call(hostdir, root);
        this._hostDirCache[rooturl] = hostdir;
      }

      cx.hostdir = hostdir;
      cx.filename = "/" + pathParts.slice(pathParts.length - i).join(JSEXT_config.sep);
      //  cx.responseLine="200 OK";

    this._execScript();
    stdout.close();
  },
  _hostDirCache: {},


  _get_request_headers: function() {
    const headers = {};
    for(var i in environment) {
      if(i.substr(0,5) != "HTTP_") continue;
      var mangled = i.substr(5).toLowerCase().replace(/_./g, function(a) {
          return a.toUpperCase().substr(1);
        });
      headers[mangled] = environment[i];
    }

    if(environment.CONTENT_TYPE) headers.contentType = environment.CONTENT_TYPE;
    if(environment.CONTENT_LENGTH) headers.contentLength = environment.CONTENT_LENGTH;
    return headers;
  },


  /*
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

     * _responseHeaders_: An object containing response headers to be sent.

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
  */
  _execScript: function() {
      var filename = this.filename;

      var pathparts = filename.split(JSEXT_config.sep);
      var curdir = this.hostdir;
      for(var i = 1; i < pathparts.length - 1; i++) curdir = curdir[pathparts[i]];

      var onlyFilename = JSEXT1.filename(filename);

      const func = load.call(curdir, curdir.$path + JSEXT_config.sep + onlyFilename);
      if(typeof func != "function") return;

      this._exec_page_function(func);
  },


  _exec_page_function: function(func) {
    try {
      func(this);
      setTimeout.exec();
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


  // Returns an object with the name/value pairs given by the posted data in [[stdin]]
  _get_POST_data: function() {
    const cx = this;
    if(cx.method != "POST") return {};
    var ct = mime.nameValuePairDecode(cx.requestHeaders.contentType);
    if(ct == "text/JSON") return decodeJSON(stdin.read()) || {};
    if(ct == "application/x-www-form-urlencoded") return http.decodeQry(stdin.read()) || {};
    if(ct == "multipart/form-data") {
      var stream = new StringFile(stdin.read()); // Sorry, must read into ram because readln is not bin-safe.
      return mime.decodeMultipart(stream, ct.boundary) || {};
    }
    return {};
  },


  _parse_cookie_header: function(cookie_header) {
    const r = {};
    if(!cookie_header) return r;
    const parts = cookie_header.split(/;\s+/);
    for(var i = 0; i != parts.length; i++) {
      var kv = parts[i].split('=');
      r[kv[0]] = kv[1] || '';
    }
    return r;
  },


  _create_context: function() {
    const cx = { __proto__: this._context_proto };
    cx._response_cookie
  },


  set_cookie: function(name, value, options) {
    this.response_cookies[name] = { value: value, __proto__: options }
  },

  _set_cookie_too_late: function() {
    throw new Error("Too late to set cookie after writing to page");
  },

  delete_cookie: function(name) {
    // xxx should maybe do nothing if there is no matching request cookie
    // Yes, in HTTP you delete cookies by forcing them to expire
    this.response_cookies[name] = { value: 'delete', expires: new Date(0) };
  },

  output_cookies: function(stream) {
    this.set_cookie = this._set_cookie_too_late;
    for(var name in this.response_cookies) {
      var c = this.response_cookies[name];
      var h = "Set-Cookie: " + name + "=" + c.value;
      if(c.expires && c.expires instanceof Date) h += "; expires=" + c.expires.toUTCString();
      if(c.path) h += "; path=" + c.path;
      if(c.domain) h += "; domain=" + c.domain;
      stream.write(h + "\r\n");
    }
  },
};

return CGI;

})()

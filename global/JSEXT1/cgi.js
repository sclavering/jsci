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

/*
A nonce type used in CGI.GET_data and CGI.POST_data so that an object arising
from dots in the submitted field names can be distinguished easily from e.g.
File-like objects that come from <input type="file"/> (by using instanceof).
*/
function FormData() {
}


function CGI(refresh) {
  this.requestHeaders = this._get_request_headers();

  this.remoteAddress = environment.REMOTE_ADDR;
  this.method = environment.REQUEST_METHOD;
  this.requestURL = "http://" + (this.requestHeaders.host || '') + environment.REQUEST_URI;

  this.GET_data = this._reinterpret_form_data(url.parse(this.requestURL).qry || {});
  this.POST_data = this._get_POST_data();
  this.cookie_data = this._parse_cookie_header(this.requestHeaders.cookie);

  this.response_cookies = {};
  // Set a default content type.
  this.responseHeaders = { contentType: 'text/html' };

  this.run(refresh);
}

CGI.prototype = {
  /*
  Called when an uncaught exception occcurs within the CGI page, and used for
  logging or debugging.  Pages should assign a function to .onerror as one of
  the first things they do.
  
  Any exceptions occurring in .onerror() will be ignored.
  
  The default implementation does nothing with the exception.
  */
  onerror: function() {
  },


  run: function(refresh) {
    const cx = this;

    stdout = new FlashWriter(stdout, function(stream) {
      stdin.close();
      if(cx.responseHeaders.location) cx.responseLine = "302 Found";
      if(cx.responseLine) stream.write("Status: " + cx.responseLine + "\r\n");
      cx.output_cookies(stream);
      mime.writeHeaders(stream, cx.responseHeaders);
      delete cx.responseHeaders;
    });

    var filename = environment.PATH_TRANSLATED || environment.SCRIPT_FILENAME;
    var path = $curdir.path(filename);
    var onlyFilename = $curdir.filename(filename);
    var urlpath = environment.PATH_INFO || environment.SCRIPT_NAME;
    var pathParts = path.split(JSEXT_config.sep);
    var urlParts = this.requestURL.split("/");
    urlParts.pop();

    var i;
    for(i = 1; i <= urlParts.length; i++) {
      if(pathParts[pathParts.length - i] != urlParts[urlParts.length - i]) break;
    }

    var root = pathParts.slice(0, pathParts.length - i + 1).join(JSEXT_config.sep);
    var rooturl = urlParts.slice(0, urlParts.length - i + 1).join("/");
    if(rooturl != "/") rooturl += "/";

    pathParts.push(onlyFilename);
    var relFilename = JSEXT_config.sep + pathParts.slice(pathParts.length - i).join(JSEXT_config.sep);

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

    this.hostdir = hostdir;
    this.filename = "/" + pathParts.slice(pathParts.length - i).join(JSEXT_config.sep);
    // this.responseLine = "200 OK";

    var filename = this.filename;

    var pathparts = filename.split(JSEXT_config.sep);
    var curdir = this.hostdir;
    for(var i = 1; i < pathparts.length - 1; i++) curdir = curdir[pathparts[i]];

    var onlyFilename = JSEXT1.filename(filename);

    const func = load.call(curdir, curdir.$path + JSEXT_config.sep + onlyFilename);
    if(typeof func != "function") return;

    this._exec_page_function(func);

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


  _exec_page_function: function(func) {
    try {
      func(this);
      setTimeout.exec();
    } catch(x if x == this._finish_token) {
      // do nothing, since we're just using this exception for control flow
    } catch(x) {
      try {
        this.onerror(x);
      } catch(e) {
      }
    }
  },


  /*
  Helper for use in .onerror().  Logs the exception to a specified log file.
  */
  log_error: function(exception, logfile_path) {
    const f = new File(logfile_path, 'a');
    f.write(this._format_error_as_string(exception));
    if(f) f.close();
  },


  /*
  For use as .onerror() when debugging.  Outputs the error in the response body (and sets the content-type to text for readability).
  */
  print_error: function(exception) {
    this.responseHeaders.contentType = 'text/plain; charset=UTF-8';
    print(this._format_error_as_string(exception));
  },


  _format_error_as_string: function(exception) {
    const x = exception;
    var s = "";
    s += new Date().toLocaleFormat('%Y-%m-%d %T') + ' ' + (Date.now() % 1000) + ":\n";
    s += '  ' + String(x) + '\n';
    if(x.fileName && x.lineNumber) s += '  Line ' + x.lineNumber + ' in ' + x.fileName + ':\n';
    if(x.stack) s += '    ' + x.stack.replace('\n', '\n    ', 'g') + '\n';
    return s;
  },


  // Returns an object with the name/value pairs given by the posted data in [[stdin]]
  _get_POST_data: function() {
    const cx = this;
    if(cx.method != "POST") return {};
    var ct = mime.nameValuePairDecode(cx.requestHeaders.contentType);
    if(ct == "text/JSON") return decodeJSON(stdin.read()) || {};
    if(ct == "application/x-www-form-urlencoded") return this._reinterpret_form_data(url.parse_query(stdin.read()) || {});
    if(ct == "multipart/form-data") {
      var stream = new StringFile(stdin.read()); // Sorry, must read into ram because readln is not bin-safe.
      return this._reinterpret_form_data(mime.decodeMultipart(stream, ct.boundary) || {});
    }
    return {};
  },


  // url.parse_query does the basic parsing of the foo=bar part of a query string, but only special-cases foo[]
  // Here we reinterpret names data like { 'foo.42.bar': 'quux' } to { foo: { 42: { bar: 'quux' }}}.
  // This is done here because we also want it to apply to multipart/form-data form submissions.
  _reinterpret_form_data: function(data) {
    const res = new FormData();
    for(var key in data) this._reinterpret_form_value(key.split("."), data[key], res);
    return res;
  },

  _reinterpret_form_value: function(name_bits, val, obj) {
    const last_bit = name_bits.pop();
    for each(var name_bit in name_bits) {
      if(name_bit in Object.prototype) return; // avoid replacing __proto__ and suchlike
      // We use instanceof rather than e.g. typeof because we don't want to add fields to an uploaded file object or similar
      if(!(name_bit in obj) || !(obj[name_bit] instanceof FormData)) obj[name_bit] = new FormData();
      obj = obj[name_bit];
    }
    if(last_bit in Object.prototype) return;
    obj[last_bit] = val;
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

  finish: function() {
    throw this._finish_token;
  },
  _finish_token: {},

  redirect: function(url) {
    this.responseHeaders.location = url;
    this.finish();
  },

  set_cookie: function(name, value, options) {
    this.response_cookies[name] = { value: value, __proto__: options }
  },

  _set_cookie_too_late: function() {
    throw new Error("Too late to set cookie after writing to page");
  },

  delete_cookie: function(name, path, domain) {
    // xxx should maybe do nothing if there is no matching request cookie
    // Yes, in HTTP you delete cookies by forcing them to expire
    this.response_cookies[name] = { value: 'delete', expires: new Date(0), path: path, domain: domain };
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


/*
A File-like object used for CGI output that flushes the http response headers
as soon as the first write() call for the response body occurs (or before
closing, if there is no response body).

We don't expose this beyond the CGI module.
*/
function FlashWriter(file, headerFunc) {
  this.file = file;
  this.headerFunc = headerFunc;
}

FlashWriter.prototype = {
  write: function(str) {
    if(this.headerFunc) {
      this.headerFunc(this.file);
      delete this.headerFunc;
    }
    return this.file.write(str);
  },

  flush: function() {
    return this.file.flush();
  },

  close: function() {
    if(this.headerFunc) {
      this.headerFunc(this.file);
      delete this.headerFunc;
    }
    this.file.close();
  },
};


CGI.FormData = FormData;

return CGI;

})()

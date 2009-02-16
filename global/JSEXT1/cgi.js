/*
new CGI()

Interprets the environment variables given in _environment_ according to the
CGI/1.1 specification: <http://hoohoo.ncsa.uiuc.edu/cgi/env.html>.

The .jsx file to be executed should contain a single anonymous function, which
will be called with the _CGI_ instance as its only argument.

### Arguments ###

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



function CGI() {
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

  this.run();
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


  run: function() {
    const cx = this;

    stdout = new FlashWriter(stdout, function(stream) {
      stdin.close();
      if(cx.responseHeaders.location) cx.responseLine = "302 Found";
      if(cx.responseLine) stream.write("Status: " + cx.responseLine + "\r\n");
      cx.output_cookies(stream);
      cx._write_headers(stream, cx.responseHeaders);
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
    const ct = this._mime_name_value_pair_decode(this.requestHeaders.contentType);
    if(ct == "text/JSON") return decodeJSON(stdin.read()) || {};
    if(ct == "application/x-www-form-urlencoded") return this._reinterpret_form_data(url.parse_query(stdin.read()) || {});
    if(ct == "multipart/form-data") {
      return this._reinterpret_form_data(this._decode_multipart_mime(stdin.read(), ct.boundary) || {});
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



  /*
  _write_headers(stream, obj)
  */
  _write_headers: function(conn, headers) {
    var lines = this._encode_headers(headers);
    for(var i = 0; i < lines.length; i++) {
      var line = lines[i].replace(/(.{78,998}) /g, "$1\r\n "); // folding
      conn.write(line + "\r\n");
    }
    conn.write("\r\n");
  },


  _encode_headers: function(obj) {
    var ret = [];
    const hasOwnProperty = Object.prototype.hasOwnProperty;
    for(var i in obj) {
      if(hasOwnProperty.call(obj, i)) {
        var e = i.substr(1);
        e = e.replace(/([A-Z])/g, "-$1");
        var line = i.substr(0, 1).toUpperCase() + e + ": " + obj[i];
        ret.push(line);
      }
    }
    return ret;
  },


  /*
  _decode_multipart_mime(str, boundary)
  
  Decodes a mime multipart message.

  ### Arguments ###

  * _boundary_: A string containing the boundary between objects
  
  ### Return value ###
  
  An object containing properties by the same names as the parts of the mime
  message. Files that are encoded in the message are returned as CGI.UploadedFile
  objects. Objects are decoded as strings unless they have Content-Type
  "text/JSON", in which case they are decoded as JSON objects.
  */
  _decode_multipart_mime: function(str, boundary) {
    var stream = new CGIStringFile(str);

    var ret = {};

    stream.readline(); // "--"+boundary;
    if(stream.eof()) return ret;

    for(;;) {
      var headers = this._read_headers(stream);
      var cd = this._mime_name_value_pair_decode(headers.contentDisposition);
      var name = cd.name;

      var buf = "";
      while(!stream.eof()) {
        var line = stream.readline();
        if(line.substr(2, boundary.length) == boundary) break;
        buf += line;
      }

      buf = buf.substr(0, buf.length - 2);

      var val;
      if(cd.filename) {
        val = new UploadedFile(buf);
        val.name = cd.filename;
      } else if(headers.contentType == "text/JSON") {
        val = decodeJSON(buf);
      } else {
        val = buf;
      }

      if(name.substr(-2) == "[]") {
        name = name.slice(0, -2);
        if(!ret[name]) ret[name]=[];
        ret[name].push(val);
      } else {
        ret[name] = val;
      }

      if(stream.eof() || line.substr(-4) == "--\r\n") break;
    }

    return ret;
  },


  /*
  _mime_name_value_pair_decode(str)

  Returns a String object with properties corresponding to the attributes of
  the mime string.

  ### Example ###

      _mime_name_value_pair_decode( 'Hello; a="b"; c="d"' )

  returns a String object whose value is "Hello", and with the properties:

    * _a_: "b"
    * _c_: "d"
  */
  _mime_name_value_pair_decode: function(str) {
    if(!str) str = '';
    var ret = new String(gettoken());
    while(str != "") {
      var name = gettoken();
      var value = gettoken();
      ret[name] = value;
    }
    return ret;

    function gettoken() {
      str = str.replace(/^[()<>@,;:\\\[\]?={} \t]*/, ""); // remove whitespace
      if(str.substr(0, 1) == '"') {
        var endQuote = str.indexOf('"', 1);
        if(endQuote == -1) endQuote = str.length;
        var ret=str.substr(1, endQuote - 1);
        str = str.substr(endQuote + 1);
        return ret;
      }
      var ret=str.match(/^[^()<>@,;:\\\[\]?={} \t]*/)[0];
      str = str.substr(ret.length);
      return ret;
    }
  },


  /*
  _read_headers(stream)

  Reads headers from a stream, i.e. until eof or an empty line.
  Lines should be terminated by chr(13) chr(10).
  */
  _read_headers: function(conn) {
    var lines = [];
    var line;
    while(!conn.eof() && (line = conn.readline()) != "\r\n") {
      if(line.substr(0,1) == " ") { // folding
        lines[lines.length - 1] += line.substr(0, line.length - 2);
      } else {
        lines.push(line.substr(0, line.length - 2));
      }
    }
    return this._read_headers_helper(lines);
  },


  /*
  _read_headers_helper(lines)

  Decodes a block of mime headers. Mime headers are of the form

      Name-name: Value

  ### Arguments ###

  * _lines_: An array containing lines
  
  ### Return value ###
  
  An object of name/value pairs. Mime names are case-insensitive,
  so the case is normalized: Nnames are first converted to all lower case.
  Letters immediately following hyphens are then converted to upper case
  and the hyphen is removed. Example:

      ["Content-Type: text/html"]

  becomes...

      { contentType: "text/html" }
  */
  _read_headers_helper: function(lines) {
    var ret = {};
    for(var i = 0; i < lines.length; ++i) {
      var line = lines[i];
      var colonpos = line.indexOf(':');
      if(colonpos == -1) throw new Error("_readHeaders: Malformed document");
      var key = line.substr(0, colonpos).toLowerCase().replace(/-./g, function(a) { return a.toUpperCase().substr(1) });
      ret[key] = line.substr(colonpos + 2);
    }
    return ret;
  },
};



function UploadedFile(data) {
  this.data = this.str = data;
  this.read = function() { return this.data; };
  this.close = function() {};
  this.name = null;
}



function CGIStringFile(str) {
  this._str = str || "";
  this._where = 0;
}

CGIStringFile.prototype={
  // Returns true if end-of-file has been reached.
  eof: function() {
    return this._where >= this._str.length;
  },

  // read(n): n bytes or to EOF.  read(): read to EOF
  _read: function(size) {
    if(arguments.length < 1) size = -1;
    if(size < 0) {
      var ret = this._str.substr(this._where);
      this._where = this._str.length;
    } else {
      var ret = this._str.substr(this._where, size);
      this._where += size;
    }
    return ret;
  },

  // read a line, returning it including the \n
  readline: function() {
    var nextpos=this._str.indexOf('\n', this._where);
    if(nextpos == -1) return this._read();
    var ret = this._str.substr(this._where, nextpos - this._where + 1);
    this._where = nextpos + 1;
    return ret;
  },
}




CGI.FormData = FormData;
CGI.UploadedFile = UploadedFile;

return CGI;

})()

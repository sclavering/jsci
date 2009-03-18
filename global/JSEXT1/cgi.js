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
  const url = this.requestURL = "http://" + (this.requestHeaders.host || '') + environment.REQUEST_URI;

  // remember: the #foo part isn't sent to the server, so we don't need to exclude it
  const qstr_ix = url.indexOf('?');
  const qstr = qstr_ix == -1 ? '' : url.slice(qstr_ix + 1);

  this.GET_data = this._reinterpret_form_data(this._parse_query(qstr) || {});
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


  respond: function(status_code, headers_dict, body_str) {
    const out = this._real_stdout;
    out.write("Status: " + status_code + "\r\n");
    for(var h in headers_dict) out.write(h + ": " + headers_dict[h] + "\r\n");
    out.write("\r\n");
    if(body_str) out.write(body_str);
    throw this._exit_nonce;
  },
  _exit_nonce: {},


  run: function() {
    const cx = this;

    this._real_stdout = stdout;

    stdout = new FlashWriter(stdout, function(stream) {
      stdin.close();
      if(cx.responseHeaders.location) cx.responseLine = "302 Found";
      if(cx.responseLine) stream.write("Status: " + cx.responseLine + "\r\n");
      cx.output_cookies(stream);
      cx._write_headers(stream, cx.responseHeaders);
      delete cx.responseHeaders;
    });

    const filename = environment.PATH_TRANSLATED || environment.SCRIPT_FILENAME;
    const func = load.call({}, filename);
    this._exec_safely(func, this, true);
    stdout.close();
  },


  // Run the page or the error handler, trapping exceptions.  (Both need .respond() and .redirect() to work.)
  _exec_safely: function(func, arg, run_onerror) {
    try {
      func.call(this, arg);
    }
    // This "exception" is just a nonce used for control flow
    catch(x if x == this._finish_token) {
      // do nothing
    }
    // This "exception" is just a nonce used for control flow
    catch(x if x == this._exit_nonce) {
      // because we don't want the FlashWriter to flush its headers
      stdout = this._real_stdout;
    }
    catch(ex) {
      if(run_onerror) this._exec_safely(this.onerror, ex, false);
    }
  },


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
  Helper for use in .onerror().  Logs each exception to a separate file whose name is |logfile_prefix| followed by a timestamp.
  */
  log_error: function(exception, logfile_prefix) {
    const timestamp = new Date().toLocaleFormat('%Y-%m-%d-%T') + '-' + (Date.now() % 1000);
    const f = new File(logfile_prefix + '-' + timestamp, 'a');
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
    if(ct == "application/x-www-form-urlencoded") return this._reinterpret_form_data(this._parse_query(stdin.read()) || {});
    if(ct == "multipart/form-data") {
      return this._reinterpret_form_data(this._decode_multipart_mime(stdin.read(), ct.boundary) || {});
    }
    return {};
  },


  // ._parse_query does the basic parsing of the foo=bar part of a query string, but only special-cases foo[]
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


  // Decodes a multipart/form-data POST body into an object, with a key for each part name, and values as either strings or CGI.UploadedFile objects (as appropriate).
  _decode_multipart_mime: function(message_str, boundary_str) {
    var str = String(message_str), boundary = String(boundary_str);

    const stream = {
      _where: 0,
      _str: str,
      eof: function() {
        return this._where >= this._str.length;
      },
      // read a line, returning it including the \n
      readline: function() {
        const oldwhere = this._where;
        const ix = this._str.indexOf('\n', oldwhere);
        this._where = ix == -1 ? this._str.length : ix + 1;
        return this._str.slice(oldwhere, this._where);
      },
    };

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
  so the case is normalized: names are first converted to all lower case.
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


  /*
  obj = x._parse_query(str)
  
  Parses a query string of the form "a=b&c=d&e", and returns an object like { a: "b", c: "d", e: null }.
  
  Where a key exists more than once, the last occurrence is used.
  */
  _parse_query: function(qry) {
    if(qry == undefined) return;
    const get = {};
    const vars = qry.split("&");
    for(var i = 0; i != vars.length; ++i) this._add_form_var_to(this._urldecode(vars[i]), get);
    return get;
  },


  // Similar to [[decodeURIComponent]], but also decodes + characters to spaces.
  _urldecode: function(qry) {
    if(qry === undefined) return;
    qry = qry.replace(/\+/g, " ");
    qry = qry.replace(/%../g, function(nn) { return String.fromCharCode(parseInt(nn.substr(1), 16)); });
    return qry;
    //    return decodeURIComponent(qry.replace(/\+/g," "));
  },


  // Interpret a single part of a query string, e.g. foo=bar, and set property object.foo = "bar"
  // If the "foo" ends in [], the object.foo is created an array, and "bar" becomes a value in that array.
  // The CGI class handles more complex behavior, like interpreting the dots in a name like 'foo.bar.baz'
  _add_form_var_to: function(name_and_value, obj) {
    const eq_ix = name_and_value.indexOf('='), has_eq = eq_ix !== -1;
    var full_name = has_eq ? name_and_value.slice(0, eq_ix) : name_and_value;
    const val = has_eq ? name_and_value.slice(eq_ix + 1) : '';
    const is_array_var = /\[\]$/.test(full_name);
    if(is_array_var) full_name = full_name.slice(0, full_name.length - 2);

    // Don't allow __proto__ or anything like that to be replaced
    if(full_name in Object.prototype) return;

    if(is_array_var) {
      // note: use |new Array| here rather than just [] so that the instanceof works
      if(!((full_name in obj) && obj[full_name] instanceof Array)) obj[full_name] = new Array();
      obj[full_name].push(val);
    } else {
      obj[full_name] = val;
    }
  },
};



function UploadedFile(data) {
  this.data = this.str = data;
  this.read = function() { return this.data; };
  this.close = function() {};
  this.name = null;
}



CGI.FormData = FormData;
CGI.UploadedFile = UploadedFile;

return CGI;

})()

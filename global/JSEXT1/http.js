({
  /*
  obj = http.get(url [, headers [, options]])
  
  Submits an HTTP/1.1 request and returns the response.
  
  ### Arguments ###
  
  * _url_: A string
  * _headers_: An object where properties should be strings and where
  property names become header names. You should use lower camel-case
  instead of hyphens for the property names. Example: If you
  want to send a "Content-Type" header, then _headers_ should contain
  a property named "contentType".
  * _options_: An object containing any of these properties:
  * _followRedirect_: Default true
  * _proxyHost_: String
  * _proxyPort_: Number
  
  ### Return value ###
  
  An object containing the following properties:
  
  * _stream_: A file-like object containing the body of the response
  * _headers_: An object with properties corresponding to received
  HTTP headers, converted to lower camel-case.
  * _statusLine_: A string containing the first line returned by the server
  */
  get: function(url, headers, options) {
    return this.request("GET", url, headers, undefined, options || {});
  },


  /*
  obj = http.post(url [, params, [, headers [, options]]])
  
  Submits an HTTP/1.1 request and returns the response.
  
  ### Arguments ###
  
  * _url_: A string
  * _params_: An object containing properties to use as form elements.
  Property names become element names. Properties that are strings
  are sent as-is. Properties that are files are sent as file elements,
  and all other data types are encoded using JSON encoding and
  given the content-type "text/JSON".
  * _headers_: An object where properties should be strings and where
  property names become header names. You should use lower camel-case
  instead of hyphens for the property names. Example: If you
  want to send a "Content-Type" header, then _headers_ should contain
  a property named "contentType".
  * _options_: An object containing any of these properties:
  * _followRedirect_: Default true
  * _proxyHost_: String
  * _proxyPort_: Number
  
  ### Return value ###
  
  An object containing the following properties:
  
  * _stream_: A file-like object containing the body of the response
  * _headers_: An object with properties corresponding to received
  HTTP headers, converted to lower camel-case.
  * _statusLine_: A string containing the first line returned by the server
  */
  post: function(url, form, headers, options) {
    options = options || {};
    headers = headers || {};
    var boundary = Math.floor(Math.random() * 1e42).toString(36);
    headers.contentType = "multipart/form-data; boundary=" + boundary;
    //headers.transferEncoding='chunked';
    function writeDoc(stream) {
      JSEXT1.mime.encodeMultipart(form, stream, boundary);
    }
    return this.request("POST", url, headers, writeDoc, options);
  },


  /*
  obj = http.request(method, url, headers, body, options)
  
  Used by [[$curdir.get]] and [[$curdir.post]] to send a request.
  
  ### Arguments ###
  
  * _method_: "GET" or "POST"
  * _url_: String
  * _headers_: See [[$curdir.get]] or [[$curdir.post]].
  * _body_: a function which will be called with one argument,
    a file-like object, to which it is supposed to write the
    document body.
  * _options_: See [[$curdir.get]] or [[$curdir.post]].
  
  ### Return value ###
  
  See [[$curdir.get]] or [[$curdir.post]].
  */
  request: function(method, url, headers, body, options) {
    headers = headers || {};
    if(!('followRedirect' in options)) options.followRedirect = true;

    const self = this;
    var redirect = [];
    var parts;
    for(var i = 0; i < 5; ++i) {
      parts = JSEXT1.url.parse(url);
      var ret = tryagain();
      if(ret) {
        // check authentication things
        if(ret.redirect) {
          redirect.push(ret.redirect);
          url = ret.redirect;
          continue;
        }
        else if(ret.authenticate) {
          if(!parts.username || !parts.password) return ret;
          if(ret.authenticate == "Digest") {
            var H = JSEXT1.md5;
            function KD(secret, data) {
              return H(secret + ':' + data);
            }
            var A1 = parts.username + ":" + ret.authenticate.realm + ":" + parts.password;
            var A2 = method + ":" + parts.fullPath;
            headers.authorization = 'Digest username="' + parts.username + '", ' + 'realm="' + ret.authenticate.realm + '", ' + 'nonce="' + ret.authenticate.nonce + '", ' + 'uri="' + parts.fullPath + '", ' + 'opaque="' + ret.authenticate.opaque + '", ' + 'response="' + KD(H(A1), ret.authenticate.nonce + ":" + H(A2)) + '"';
          } else if(ret.authenticate == "Basic") {
            headers.authorization = "Basic " + JSEXT1.encodeBase64(parts.username + ":" + parts.password);
          }
          ret = tryagain();
          if(ret) return ret;
        } else {
          return ret;
        }
      }
    }

    throw new Error("get: Too many redirects");

    function tryagain() {
      if(parts.port) {
        headers.host = parts.host + ":" + parts.port;
      } else {
        headers.host = parts.host;
        parts.port = 80;
      }

      if(options.proxyHost) {
        var host = parts.proxyHost;
        var port = parts.proxyPort || 80;
      } else {
        var host = parts.host;
        var port = parts.port;
      }

      if(body) {
        if(typeof body === "function") {
          headers.transferEncoding = "chunked";
        } else {
          headers.contentLength = String(body.length);
        }
      } else {
        headers.contentLength = "0";
      }

      var ret = self.connectionPool(host, port, method + " " + parts.fullPath + " HTTP/1.1\r\n" + JSEXT1.mime.encode(headers).join("\r\n") + "\r\n\r\n", body, options);

      if(ret.headers.transferEncoding == "chunked") {
        var oldclose = ret.stream.close;
        ret.stream.close = function() {
          oldclose.call(this);
          ret.trailers = JSEXT1.mime.readHeaders(this);
        }
      }

      switch(ret.statusLine[9]) {
        case '1':
        case '2':
          // ok
          break;
        case '3':
          if(options.followRedirect) {
            // redirect
            ret.stream.close();
            ret.redirect = ret.headers.location;
            break;
          }
          break;
        case '4':
          if(ret.headers.wwwAuthenticate) {
            ret.authenticate = JSEXT1.mime.nameValuePairDecode(ret.headers.wwwAuthenticate);
            break;
          }
        case '5':
          // error
          ret.error = ret.statusLine.substr(9);
          break;
      }

      if(ret.error) throw new Error("request: " + ret.error);
      ret.redirectHistory = redirect;
      return ret;
    }
  },


  /*
  obj = readMessage(conn)
  
  Reads an HTTP message. The first line is a status line.
  Following lines are header fields until the first empty line.
  The remaining message is not read. Instead, a stream is
  returned which, when read, will read and decode the message
  body. The possible decoding filters are:
  
  * [[$curdir.ChunkReader]] for chunked encoding
  * [[$curdir.LengthReader]] for messages with a Content-Length header field.
  * [[$curdir.LengthReader]] for messages without a body (GET messages).
  * [[$curdir.ReaderOnly]] for other messages.
  
  ### Arguments ###
  
  * _conn_: A file-like object, connected to an HTTP server
  
  ### Return value ###
  
  An object containing the following properties:
  
  * _statusLine_: The first line from the server, without trailing newline.
  * _headers_: An object containing HTTP headers returned from the
    server, converted to lower camel-case.
  * _stream_: A file-like object which can be used to read the message
    body.
  */
  readMessage: function(conn) {
    var sl = conn.readline();

    // this happens when connection is closed since last use
    if(sl == "") return; // throw new Error("http: No status line");

    var ret = {
      statusLine: sl.replace(/\r\n$/, ""),
      headers: JSEXT1.mime.readHeaders(conn),
    };

    if(ret.headers.transferEncoding == "chunked") {
      ret.stream = new this.ChunkReader(conn);
    } else if(ret.headers.contentLength) {
      ret.stream = new this.LengthReader(conn, ret.headers.contentLength);
    } else if(sl.substr(0,3) == "GET") {
      ret.stream = new this.LengthReader(conn,0);
    } else {
      ret.stream = new this.ReaderOnly(conn);
    }
    return ret;
  },


  /*
  connectionPool(host, port, header, body, options)
  
  Sends _header_ and _body_ to
  _host:port_, using a pooled connection if possible.
  
  Returns response from [[$curdir.readMessage]].
  */
  _maxconn: 4,
  _queue: [],
  _pool: {},
  connectionPool: function(host, port, header, body, options) {
    if(this._pool[host + ':' + port]) {
      try {
        var conn = this._pool[host + ':' + port];
        conn.write(header);
        if(body) {
          if(typeof body === "function")
            body(conn);
          else
            conn.write(body);
          conn.write("\r\n"); // footers
        }
        conn.flush();
        return this.readMessage(conn);
      } catch(x) {
        conn.close();
        for(var i= 0; i < this._queue.length; ++i) {
          if(this._queue[i] == host + ':' + port) this._queue.splice(i, 1);
        }
        delete this._pool[host + ':' + port];
      }
    }

    if(this._queue.length == this._maxconn) {
      var oldconn = this._queue.shift();
      this._pool[oldconn].close();
      delete this._pool[oldconn];
    }

    var conn = JSEXT1.tcp.connect(host, port);

    this._pool[host + ':' + port] = conn;
    this._queue.push(host + ':' + port);

    conn.write(header);

    switch(typeof body) {
      case 'function':
        var chunkStream = new this.ChunkWriter(conn);
        body(chunkStream);
        chunkStream.close();
        conn.write("\r\n"); // footers
        break;
      case 'undefined':
        break;
      default:
        conn.write(body);
    }
    conn.flush();
    return this.readMessage(conn);
  },
})

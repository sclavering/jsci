/*
Contains various functions encode and decode mime headers and messages.
See [RFC 2045].

[RFC 2045]: http://www.faqs.org/rfcs/rfc2045
*/
({
  /*
  _readHeaders(lines)

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
  _readHeaders: function(lines) {
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
  encode( obj )
  
  Performs the exact opposite of _decode_.
  */
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
  readHeaders(stream)

  Reads headers from a stream, i.e. until eof or an empty line.
  Lines should be terminated by chr(13) chr(10).
  */
  readHeaders: function(conn) {
    var lines = [];
    var line;
    while(!conn.eof() && (line = conn.readline()) != "\r\n") {
      if(line.substr(0,1) == " ") { // folding
        lines[lines.length - 1] += line.substr(0, line.length - 2);
      } else {
        lines.push(line.substr(0, line.length - 2));
      }
    }
    return mime._readHeaders(lines);
  },


  /*
  writeHeaders(stream, obj)

  Does the precise opposite of [[$curdir.readHeaders]].
  */
  writeHeaders: function(conn, headers) {
    var lines = mime._encode_headers(headers);
    for(var i = 0; i < lines.length; i++) {
      var line = lines[i].replace(/(.{78,998}) /g, "$1\r\n "); // folding
      conn.write(line + "\r\n");
    }
    conn.write("\r\n");
  },


  /*
  decodeMultipart(stream, boundary)
  
  Decodes a mime multipart message.

  ### Arguments ###

  * _stream_: A stream to read objects from
  * _boundary_: A string containing the boundary between objects
  
  ### Return value ###
  
  An object containing properties by the same names as the parts of the mime
  message. Files that are encoded in the message are returned as _StringFile_
  objects. Objects are decoded as strings unless they have Content-Type
  "text/JSON", in which case they are decoded as JSON objects.
  */
  decodeMultipart: function(stream, boundary) {
    var ret = {};

    stream.readline(); // "--"+boundary;
    if(stream.eof()) return ret;

    for(;;) {
      var headers = mime.readHeaders(stream);
      var cd = mime.nameValuePairDecode(headers.contentDisposition);
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
        val = new StringFile(buf);
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
  nameValuePairDecode(str)

  Returns a String object with properties corresponding to the attributes of
  the mime string.

  ### Example ###

      nameValuePairDecode( 'Hello; a="b"; c="d"' )

  returns a String object whose value is "Hello", and with the properties:

    * _a_: "b"
    * _c_: "d"
  */
  nameValuePairDecode: function(str) {
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
})

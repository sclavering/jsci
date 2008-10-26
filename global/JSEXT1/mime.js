/*
     Contains various functions encode and decode mime headers and messages.
     See [RFC 2045].

     [RFC 2045]: http://www.faqs.org/rfcs/rfc2045
    */

({

  /*
        encodeMultipart(obj, stream, boundary, [options])

    Encodes all properties of _obj_ as a multipart mime message
    and writes them to _stream_, using _boundary_ as the boundary.
    File-like properties are sent as files and closed.

    ### Options ###

    * _disposition: String, default "form-data".
    * _close_: Default true. Send end-marker (but don't close the stream)
    * _binsafe_: Default true. Stream is binary-safe. Otherwise, uses base 64-encoding

  */

  encodeMultipart:function(obj, conn, boundary, options) {
  options=options || {};
  options.disposition = options.disposition || "form-data";
  if (options.close===undefined)
    options.close=true;
  if (options.binsafe===undefined)
    options.binsafe=true;

  var hasOwnProperty=Object.prototype.hasOwnProperty;

  for (var i in obj) {
    if (hasOwnProperty.call(obj,i)) {
      conn.write("--"+boundary+"\r\n");
      switch(typeof obj[i]) {
	
      case "string":
	conn.write('Content-Disposition: '+options.disposition+'; name="'+i+'"\r\n');
	conn.write('Content-Type: text/plain\r\n');
	if (options.binsafe) {
	  conn.write("\r\n");
	  conn.write(obj[i]);
	} else {
	  conn.write("Content-Transfer-Encoding: quoted-printable\r\n\r\n");
	  conn.write(encodeQuotedPrintable(obj[i]));
	}
	conn.write("\r\n");
	break;
	
      case "object":
	if (typeof obj[i].write == 'function') {
	  conn.write('Content-Disposition: '+options.disposition+'; name="'+i+'"; filename="'+obj[i].name+'"\r\n');
	  var extension=obj[i].name.match(/\.([^.]*)$/);
	  if (extension)
	    extension=extension[1];
	  if (extension && mime.type[extension])
	    conn.write('Content-Type: '+mime.type[extension]+"\r\n\r\n");
	  else
	    conn.write('Content-Type: binary\r\n');

	  if (options.binsafe) {
	    conn.write("\r\n");
	    while (!obj[i].eof()) {
	      conn.write(obj[i].read(65536));
	    }
	  } else {
	    conn.write("Content-Transfer-Encoding: base64\r\n\r\n");
	    while (!obj[i].eof()) {
	      conn.write(encodeBase64(obj[i].read(65536)));
	    }
	  }
	  obj[i].close();
	  conn.write("\r\n");
	  
	  break;
	}

	// fall through

      case "number":  
      case "array":
      case "null":
      default:  // why doesn't opera do this with number...
	conn.write('Content-Disposition: '+options.disposition+'; name="'+i+'"\r\n');
	conn.write('Content-Type: text/JSON\r\n\r\n');
	conn.write(encodeJSON(obj[i]));
	conn.write("\r\n");
	break;
	
	
      }
    }
  }
  
  if (options.close)
    conn.write("--"+boundary+"--\r\n");
},

/*
         decode(lines)

     Decodes a block of mime headers. Mime headers are of the
     form

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

         {contentType: "text/html"}

*/

decode:function(lines) {
  var ret={};

  for (var i=0; i<lines.length; i++) {
    var line=lines[i];
    
    var colonpos=line.indexOf(':');
    if (colonpos==-1)
      throw new Error("decode: Malformed document");
    
    ret[line.
	substr(0,colonpos).
	toLowerCase().
	replace(/-./g,
		function(a) {  return a.toUpperCase().substr(1) }
		)
       ]=line.substr(colonpos+2);
  }

  return ret;
},

/*
         encode( obj )

     Performs the exact opposite of _decode_.
*/

encode:function(obj) {
  var ret=[];
  var hasOwnProperty=Object.prototype.hasOwnProperty;

  for (var i in obj) {
    if (hasOwnProperty.call(obj, i)) {
      var e=i.substr(1);
      e=e.replace(/([A-Z])/g,"-$1");
      var line=i.substr(0,1).toUpperCase()+e+": "+obj[i];
      ret.push(line);
    }
  }

  return ret;
},


/* 

         readHeaders( stream )

     Reads headers from a stream, i.e. until eof or an empty line.
     Lines should be terminated by chr(13) chr(10).

     Calls [[$curdir.decode]] to decode the headers.

    */

readHeaders: function(conn) {
  var lines=[];
  var line;
  while (!conn.eof() && (line=conn.readline())!="\r\n") {
    if (line.substr(0,1)==" ") // folding
      lines[lines.length-1]+=line.substr(0,line.length-2);
    else
      lines.push(line.substr(0,line.length-2));
  }
  return mime.decode(lines);
},

/* 

         writeHeaders( stream, obj )

     Does the precise opposite of [[$curdir.readHeaders]].

    */

writeHeaders: function(conn, headers) {
    
  var lines=mime.encode(headers);
  for (var i=0; i<lines.length; i++) {
    var line=lines[i].replace(/(.{78,998}) /g,"$1\r\n "); // folding
    conn.write(line+"\r\n");
  }
  conn.write("\r\n");
},


/*

         decodeMultipart( stream, boundary )

     Decodes a mime multipart message.

     ### Arguments ###

     * _stream_: A stream to read objects from
     * _boundary_: A string containing the boundary between objects

     ### Return value ###

     An object containing properties by the same names
     as the parts of the mime message. Files that are encoded in the
     message are returned as _StringFile_ objects. Objects are decoded
     as strings unless they have Content-Type "text/JSON", in which case
     they are decoded as JSON objects.

    */

decodeMultipart:function(stream, boundary) {

  var ret={};

  stream.readline(); // "--"+boundary;
  if (stream.eof())
    return ret;

  for (;;) {
    var headers=mime.readHeaders(stream);
    var cd=mime.nameValuePairDecode(headers.contentDisposition);
    var name=cd.name;

    var buf="";
    while (!stream.eof()) {
      var line=stream.readline();
      if (line.substr(2,boundary.length)==boundary)
	break;
      buf+=line;
    }

    buf=buf.substr(0,buf.length-2);

    var val;
    if (cd.filename) {
      val=new StringFile(buf);
      val.name=cd.filename;
    } else if (headers.contentType=="text/JSON") 
      val=decodeJSON(buf);
    else
      val=buf;

    if (name.substr(-2)=="[]") {
      name = name.slice(0, -2);
      if (!ret[name])
	ret[name]=[];
      ret[name].push(val);
    } else
      ret[name]=val;

    if (stream.eof() || line.substr(-4)=="--\r\n") break;
  }

  return ret;
},

/* 
          nameValuePairDecode( str )

      Returns a String object with properties corresponding to the
      attributes of the mime string.

      ### Example ###

          nameValuePairDecode( 'Hello; a="b"; c="d"' )

      returns a String object whose value is "Hello" and which as
      the following properties:

      * _a_: "b"
      * _c_: "d"

    */

nameValuePairDecode: function(str) {
  if(!str) str = '';
  var ret=new String(gettoken());
  while (str!="") {
    var name=gettoken();
    var value=gettoken();
    ret[name]=value;
  }
  return ret;

  function gettoken() {
    str=str.replace(/^[()<>@,;:\\\[\]?={} \t]*/,""); // remove whitespace
    if (str.substr(0,1)=='"') {
      var endQuote=str.indexOf('"',1);
      if (endQuote==-1)
	endQuote=str.length;
      var ret=str.substr(1,endQuote-1);
      str=str.substr(endQuote+1);
      return ret;
    }
    var ret=str.match(/^[^()<>@,;:\\\[\]?={} \t]*/)[0];
    str=str.substr(ret.length)
    return ret;
  }
},

/*

          nameValuePairEncode( obj )

      Should be called with a String object containing string properties.

      ### Example ###

          var S=new String("Hello");
          S.a="b";
          S.c="d";
          var x=nameValuePairEncode( S );

      returns

          'Hello; a="b"; c="d"'

    */

nameValuePairEncode: function(obj) {
  var ret=String(obj);
  if (typeof(obj)=="object") {
    var i;
    for (i in obj) {
      ret+="; "+i+"="+mime.attribValueEncode(obj[i]);
    }
  }
  return ret;
},

/*
         attribValueDecode( str )

     Removes quotes around a string if they are there.
     Otherwise leaves the string untouched.

     ### Arguments ###

     * _str_: A string, possibly surrounded in quotes.

     ### Return value ###

     A string without surrounding quotes

    */

attribValueDecode: function(str) {
  if (str[0]=="\"") return str.substr(1,str.length-2);
  else return str;
},

/*
         attribValueEncode( str )

     Returns _str_, enclosed in quotes.
    */

attribValueEncode: function(str) {
  return "\""+str+"\"";
}

})

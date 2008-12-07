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

function(method, url, headers, body, options) {
  var i;
  var redirect=[];
  var parts;

  if (!('followRedirect' in options))
    options.followRedirect=true;

  headers=headers || {};

  for (i=0; i<5; i++) {
    parts = url.parse(url);

    var ret=tryagain();

    if (ret) {
      // check authentication things

      if (ret.redirect) {

	redirect.push(ret.redirect);
	url=ret.redirect;
	continue;

      } else if (ret.authenticate) {

	if (!parts.username || !parts.password)
	  return ret;

	if (ret.authenticate=="Digest") {
	  
	  var H=$parent.md5;

	  function KD(secret, data) {
	    return H(secret+':'+data);
	  }

	  var A1=parts.username+":"+ret.authenticate.realm+":"+parts.password;
	  var A2=method + ":" + parts.fullPath;

	  headers.authorization='Digest username="'+parts.username+'", '+
	    'realm="'+ret.authenticate.realm+'", '+
	    'nonce="'+ret.authenticate.nonce+'", '+
	    'uri="'+parts.fullPath+'", '+
	    'opaque="'+ret.authenticate.opaque+'", '+
	    'response="'+KD(H(A1), ret.authenticate.nonce+":"+H(A2))+'"';
	} else if (ret.authenticate=="Basic") {
	  headers.authorization="Basic "+$parent.encodeBase64(parts.username+":"+parts.password);
	}

	ret=tryagain();
	if (ret)
	  return ret;
      } else {

	return ret;

      }
    }
  }

  throw new Error("get: Too many redirects");

  function tryagain() {
    if (parts.port) {
      headers.host=parts.host+":"+parts.port;
    } else {
      headers.host=parts.host;
      parts.port=80;
    }

    if (options.proxyHost) {
      var host=parts.proxyHost;
      var port=parts.proxyPort || 80;
    } else {
      var host=parts.host;
      var port=parts.port;
    }

    if (body) {
      if (typeof body==="function")
	headers.transferEncoding="chunked";
      else
	headers.contentLength=String(body.length);
    } else {
      headers.contentLength="0";
    }

    var ret=connectionPool(host,
			   port,
			   method+" "+parts.fullPath+" HTTP/1.1\r\n"+
			   $parent.mime.encode(headers).join("\r\n")+"\r\n\r\n",
			   body,
			   options);


    if (ret.headers.transferEncoding=="chunked") {

      var oldclose=ret.stream.close;
      
      ret.stream.close=function() {
	oldclose.call(this);
	ret.trailers=$parent.mime.readHeaders(this);
      }
    }

    switch(ret.statusLine[9]) {
    case '1':
    case '2':
      // ok
      
      break;
      
    case '3':
      if (options.followRedirect) {
	// redirect
	ret.stream.close();
	ret.redirect=ret.headers.location;
	break;
      }

      break;

    case '4':
      if (ret.headers.wwwAuthenticate) {
	ret.authenticate=$parent.mime.nameValuePairDecode(ret.headers.wwwAuthenticate);
	break;
      }

    case '5':

      // error
      ret.error=ret.statusLine.substr(9);
      break;
    }
    
    if (ret.error) {
      throw new Error("request: "+ret.error);
    }

    ret.redirectHistory=redirect;
    return ret;

  }


}

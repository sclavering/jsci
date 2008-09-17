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

function(url, form, headers, options) {

  options = options || {};

  headers=headers || {};

  var boundary=Math.floor(Math.random()*1e42).toString(36);
  headers.contentType="multipart/form-data; boundary="+boundary;
  //headers.transferEncoding='chunked';

  return request("POST",
		 url,
		 headers,
		 writeDoc,
		 options);

  function writeDoc(stream) {
    $parent.mime.encodeMultipart(form, stream, boundary);
  }
}

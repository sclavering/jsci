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

function(url, headers, options) {

  options = options || {};

  return request("GET",
		 url,
		 headers,
		 undefined,
		 options);
}

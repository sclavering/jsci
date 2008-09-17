/*

    xml = http.soap(url, SOAPAction, xml2 [, options])

Submits an HTTP/1.1 request and returns the response. Uses
HTTP POST. SOAPAction is sent as an HTTP header. The xml document
is encoded using UTF-8.

### Arguments ###

* _url_: A string
* _SOAPAction_: A string. Will be enclosed in quotes and sent as
  the HTTP header _SOAPAction_.
* _xml2_: A [[String]] or [[XML]] object. Should not contain
  the &lt;?xml...?> line, since one will be added.
* _options_: An object containing any of these properties:
  * _followRedirect_: Default true
  * _proxyHost_: String
  * _proxyPort_: Number

### Return value ###

The body of the returned document in an [[XML]] object.

*/

function(url, SOAPAction, xml, options) {
  
  options = options || {};

  var ret=request("POST",
		 url,
                 {contentType: 'text/xml; charset=utf-8',
		  soapaction: '"'+SOAPAction+'"'},
		 '<?xml version="1.0" encoding="UTF-8"?>'+
		 $parent.encodeUTF8(String(xml)),
		 options);

  return $parent.soap.toXML(ret.stream.read());
}

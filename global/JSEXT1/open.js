/*
    open(uri, [...])

Opens and returns the resource identified by _uri_.
If the _uri_ does not match a regexp for a URI, then
the _uri_ parameter is interpreted as a filename.

Note: file:// is the only supported URI protocol (support for http:// and others was removed).
*/

(function(curdir){
  return function(uri) {
    uri = url.parse(uri);
    if(uri.protocol && uri.protocol != "file") throw Error("Unknown protocol handler '" + uri.protocol + "'");
    return openfile.apply(openfile, arguments);
  }

  function openfile(uri, mode) {
    return new JSEXT1.File(uri.fullPath, mode);
  }
})(this)

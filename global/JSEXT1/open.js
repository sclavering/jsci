/*
    open(uri, [...])

Opens and returns the resource identified by _uri_.
If the _uri_ does not match a regexp for a URI, then
the _uri_ parameter is interpreted as a filename.

See documentation for _open_'s properties for details
about each protocol.


*/

(function(curdir){
  return function(uri) {
    uri = url.parse(uri);

    var clas=arguments.callee[uri.protocol || "file"];
    if (!clas)
      throw Error("Unknown protocol handler '"+uri.protocol+"'");
    return clas.apply(clas, arguments);
  }
})(this)
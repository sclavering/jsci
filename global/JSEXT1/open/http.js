/*
      stream = open("http://host:port/path/file")

  Opens a connection to an http server by issuing a GET request.
 */

function(uri) {
  return $parent.http.get($parent.http.encodeURI(uri), undefined, {getStream: true}).stream;
}

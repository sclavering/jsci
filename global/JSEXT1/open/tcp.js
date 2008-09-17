/*

    stream = open("tcp://host:port/")

Opens a raw tcp connection to a server.

*/


function(uri) {
  return $parent.tcp.connect(uri.host, uri.port);
}


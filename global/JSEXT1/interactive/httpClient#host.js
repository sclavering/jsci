/*
      obj = httpClient( host, port [, options] )

  Opens a port and receives http connections which will be an interactive shell, executing commands in the client but using code fetched from the server.

  See [[$parent.http.Server]] for a description of the arguments.

  ### Return value ###

  A [[$parent.http.Server]] object.

    */

function(host, port, options) {
  return new $parent.http.Server(host, port, $path+JSEXT_config.sep+"0-rootdir-httpClient", options);
}

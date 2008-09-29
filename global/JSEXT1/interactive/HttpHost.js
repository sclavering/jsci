/*
     Opens a port and receives one http connection which will be an interactive shell to the host
*/

function(host, port, options) {
  return new $parent.http.Server(host, port, arguments.callee.$path, options);
}

/*
      smtp.connect(host [, port=25])

  Returns new [[$curdir.Connection]].

 */

function(host, port) {
  var conn=$parent.tcp.connect(host, port || 25);
  return new Connection(conn);
}

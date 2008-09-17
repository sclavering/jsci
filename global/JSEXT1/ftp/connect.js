/*
      ftp.connect(host [, port=21])

  Returns new [[$curdir.Connection]].

 */


function(host, port) {
  var conn=$parent.tcp.connect(host, port || 21);
  return new Connection(conn);
}

/*
    stream = open("ftp://host/path/file", "r")

  Opens a connection to an ftp server and downloads or uploads a file. The second argument
  can be "r" or "w".

 */

function(uri, mode) {
  if (!uri.username) {
    uri.username="anonymous";
    uri.password="ftp";
  }
  var conn=$parent.ftp.connect(uri.host, uri.port);
  conn.login(uri.username, uri.password);
  if (mode=="w")
    var file=conn.stor(uri.path);
  else
    var file=conn.retr(uri.path);

  var oldclose=file.close;
  file.close=function() {
    oldclose.call(file);
    conn.close();
  }
  return file;
}

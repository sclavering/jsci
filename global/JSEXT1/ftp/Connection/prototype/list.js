/*
      conn.list([path])

  Return an array of strings with directory listing.

 */

function(path) {
  path=path||"";

  this.sendCommand("PASV");

  var parts=this.response.match(/\(([0-9]*),([0-9]*),([0-9]*),([0-9]*),([0-9]*),([0-9]*)\)/);
  var dataconn=tcp.connect(parts[1]+"."+parts[2]+"."+parts[3]+"."+parts[4], Number(parts[5])*256+Number(parts[6]));

  this.sendCommand("LIST "+path);

  var ret=dataconn.readlines();
  dataconn.close();
  this.readResponse();
  return ret;
}

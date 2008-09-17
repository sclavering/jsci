/*
      file = conn.retr(filename)

  Retrieves a file using passive FTP. The
  return value is a file-like object which should be read and closed
  before other commands are issued on the same connection.

 */

function(filename) {
  this.sendCommand("PASV");

  var parts=this.response.match(/\(([0-9]*),([0-9]*),([0-9]*),([0-9]*),([0-9]*),([0-9]*)\)/);
  var dataconn=tcp.connect(parts[1]+"."+parts[2]+"."+parts[3]+"."+parts[4], Number(parts[5])*256+Number(parts[6]));

  this.sendCommand("TYPE I");
  this.sendCommand("RETR "+filename);

  var self=this;
  var oldclose=dataconn.close;
  
  dataconn.close=function() {
      oldclose.call(dataconn);
      self.readResponse();
  }
  
  return dataconn;
}

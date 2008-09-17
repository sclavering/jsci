/*
      number = conn.sendCommand(cmd)
  
  Sends a command to the server and reads the response. Returns
  status code.

 */
function(cmd) {
  this.conn.write(cmd+"\r\n");
  this.conn.flush();
  return this.readResponse();
}

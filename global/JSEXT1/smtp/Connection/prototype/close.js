/*

      conn.close()

  Close the connection.

 */
function() {
  if (!this.closed) {
    this.closed=true;
    
    this.sendCommand("QUIT");
    this.conn.close();
  }
}

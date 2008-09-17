/*
      conn.dele(name)

  Removes a file from the server
 */


function (name) {
    this.sendCommand("DELE "+name);
}
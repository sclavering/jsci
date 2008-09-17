/*
      conn.mkd(name)

  Makes a directory on the server
 */


function (name) {
    this.sendCommand("MKD "+name);
}
/*
      conn.rename(from, to)

  Renames a file on the server
 */


function (from, to) {
    this.sendCommand("RNFR "+from);
    this.sendCommand("RNTO "+to);
}
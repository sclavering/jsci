/*
      conn.rmd(name)

  Removes a directory from the server
 */


function (name) {
    this.sendCommand("RMD "+name);
}
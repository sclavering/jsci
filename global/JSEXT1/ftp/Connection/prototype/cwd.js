/*
      conn.cwd(name)

  Change directory
 */


function (name) {
    this.sendCommand("CWD "+name);
}
/*
      conn.login(username, password)

 */

function(username, password) {
  this.sendCommand("USER "+username);
  this.sendCommand("PASS "+password);
}

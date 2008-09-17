/*

      conn.helo(host)

  Sends a host name, which the mail server may look up to check against your IP address.

 */

function(who) {
  this.sendCommand("HELO "+who);
}

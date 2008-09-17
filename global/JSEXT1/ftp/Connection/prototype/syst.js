/*
      string = conn.syst()

  Returns the system name
*/


function () {
    this.sendCommand("SYST");
    return this.response.substr(4);
}

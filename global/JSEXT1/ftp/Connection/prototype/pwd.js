/*
      string = conn.pwd()

  Returns the working directory on the server
*/


function () {
    this.sendCommand("PWD");
    return this.response.match(/"([^"])*"/)[1];
}

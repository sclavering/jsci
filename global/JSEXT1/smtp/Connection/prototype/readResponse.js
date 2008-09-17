/*
      number = conn.readResponse();

  Reads a response from the server. Multi-line responses are stored in
  'this.message'. The last line of the response and one-line responses
  are stored in 'this.response'. The numerical value of the status
  code is returned.

  If the status code is not in the range 100-399, an Error is thrown.

 */

function() {
  for (;;) {
    var line=this.conn.readline();
    switch(line[0]) {
    case '1':
    case '2':
    case '3':
      if (line[3]=="-") {
	var stopcrit=line.substr(0,3);
	this.message="";
	for(;;) {
	  var line=this.conn.readline();
	  if (line.substr(0,3)==stopcrit)
	    break;
	  this.message+=line;
	}
      }
      this.response=line;
      return Number(line.substr(0,3));
    case '4':
    default:
      throw new Error("SMTP response:"+line.replace(/[\r\n]*/g,""));
    }
  }
}

/*

      stream = conn.mail(from, toarray)

  Sends an email. Returns a file-like object which can be written to and closed before
  issuing other commands on the connection. Headers,
  message body and attachments should be written to this file. Any occurrences of "\r\n."
  will be escaped to "\r\n..". No other encoding is performed.

  ###Arguments###

  * _from_: String. Sender of message
  * _toarray_: An array containing recipients of the message

  See [[$curdir.send]] for a higher-level and easier-to-use function.

 */

function(from, to) {
  this.sendCommand("MAIL FROM:<"+from+">");

  for (var i=0; i<to.length; i++)
    this.sendCommand("RCPT TO:<"+to[i]+">");
  this.sendCommand("DATA");

  var mente="";

  var self=this;

  return {
    write: function(block) {
      block=mente+block;
      mente="";
      block=block.replace(/\r\n?$/,function(str) {
	  mente=str;
	  return "";
	});
      block=block.replace(/\r\n\./g,"\r\n..");
      self.conn.write(block);
    },

    close: function() {
      self.conn.write(mente+"\r\n.\r\n");
      self.readResponse();
    }
  }

}

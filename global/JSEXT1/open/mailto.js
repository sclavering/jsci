/*
      stream = open("mailto:astark1@unl.edu?subject=MailTo Comments&cc=ASTARK1@UNL.EDU&bcc=id@internet.node",
		    "smtp.example.com")

  Sends an email message. If the url contains a body, it will be written to the stream before it is returned.
  The default smtp server is _localhost_. The following can be
  specified in the query part of the url:

  * _subject_: The message subject
  * _cc_: Comma-separated list of carbon-copy recipients
  * _bcc_: Comma-separated list of black carbon-copy recipients
  * _from_: Sender's address. Default is "anonymous@localhost".

 */

function(uri, smtp) {
  uri.from = uri.from || "anonymous@localhost";
  smtp = smtp || "localhost";

  var conn=$parent.smtp.connect(smtp);
  var msg=conn.send(uri);
  if (uri.body)
    msg.write(uri.body);
  return msg;
}

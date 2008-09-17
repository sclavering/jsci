/*
  
      stream = conn.send( argobj )

  Sends an email. The argument is an object, containing these properties:

  * _from_: Required string. Sender of message.
  * _to_: Optional array. Recipients of message.
  * _cc_: Optional array. Recipients of message carbon-copy.
  * _bcc_: Optional array. Recipients of message carbon-copy, not mentioned to other recipients
  * _subject_: Optional string.
  * _attachments_: Optional object containing strings and file-like objects.

  File-like attachments are closed after being sent.

  ### Return value ###

  A file-like object which the message body can be written to. The message is
  sent when this object is closed (with stream.close).

  ### Address format ###

  Addresses (from, to, cc, and bcc) can be given as bare strings, i.e.

      adr = 'john.doe@example.com'

  Or with angle brackets preceded (optionally) by a display name:

      adr = 'Doe, John <john.doe@example.com>'

  _send_ will encode display names with non-ASCII characters.


  ---

  **Example**

  ---


      var c=JSEXT1.smtp.connect("smtp.example.com");

      var msg = c.send({
	      from: 'Doe, Jane <jane.doe@example.com>',
	      to: ['Doe, John <john.doe@example.com>'],
	      subject: "Funny business",
	      attachments: {
                            att1: "A string",
		            att2: new JSEXT1.File("/tmp/file")
                           }
             });

      msg.write("Dear John,\n\nThis is my message.\n\n-- \nJane");
      msg.close();

      c.close();

  ---

  For a lower-level and more flexible function, see [[$curdir.mail]].

 */


function( argobj ) {
  var headers={};
  var to=[];

  // escape intl chars in from addr and extract what is between brackets
  headers.from = addrcpt([argobj.from]);
  var from=to.pop();

  if (argobj.to) {
    headers.to = addrcpt(argobj.to);
  }

  if (argobj.cc) {
    headers.cc = addrcpt(argobj.cc);
  }

  if (argobj.bcc) {
    addrcpt(argobj.bcc);
  }

  // Add to recipients array (to) and format the header field for the addresses.

  function addrcpt(adrs) {
    var ret=[];

    for (var i=0; i<adrs.length; i++) {
      var m=adrs[i].match(/^([^<]*)<([^>]*)>([ \t]+)?$/);
      if (m) {
	var n=m[1].replace(/^[ \t]/,"").replace(/[ \t]$/,"");
	m=m[2];
	if (n.match(/[,\u0080-\uffff]/)) {
	  n=miniescape(n).replace(/,/g,"=2C");
	}
	
	ret.push(n+" <"+m+">");
      } else {
	m=adrs[i];
	ret.push("<"+adrs[i]+">");
      }
      to.push(m);
    }

    return ret.join(", ");
  }

  // Escape intl chars in addresses and subject

  function miniescape(str) {
    return "=?UTF-8?Q?"+encodeQuotedPrintable(encodeUTF8(str)).replace(/\?/g,"=3F").replace(/_/g,"=5F").replace(/ /g,"_")+"?=";
  }

  var pstream=this.mail(from, to);

  if (argobj.subject) {
    headers.subject=argobj.subject;
    if (headers.subject.match(/[\u0080-\uffff]/))
      headers.subject=miniescape(headers.subject);
  }
  headers.mimeVersion="1.0";

  if (argobj.attachments) {
    var boundary=Math.floor(Math.random()*1e42).toString(36);
    headers.contentType="multipart/mixed; boundary=\""+boundary+"\"";
    mime.writeHeaders(pstream, headers);

    pstream.write("This is a multi-part message in MIME format.\r\n\r\n");
    pstream.write("--"+boundary+"\r\n");
    pstream.write("Content-Type: text/plain; charset=utf-8\r\n");
    pstream.write("Content-Transfer-Encoding: quoted-printable\r\n\r\n");

  } else {
    headers.contentType="text/plain; charset=utf-8";
    headers.contentTransferEncoding="quoted-printable";
    mime.writeHeaders(pstream, headers);
    
  }

  return {

  write: function(str) {
      pstream.write(encodeQuotedPrintable(encodeUTF8(str)));
    },

  close: function() {
      if (!this.closed) {
	if (argobj.attachments) {
	  pstream.write("\r\n");
	  mime.encodeMultipart(argobj.attachments, pstream, boundary, {disposition:"attachment", binsafe:false});
	}
	pstream.close();
	this.closed=true;
      }
    }
  };
}

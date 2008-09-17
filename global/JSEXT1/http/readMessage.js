/*

      obj = readMessage(conn)

  Reads an HTTP message. The first line is a status line.
  Following lines are header fields until the first empty line.
  The remaining message is not read. Instead, a stream is
  returned which, when read, will read and decode the message
  body. The possible decoding filters are:

  * [[$curdir.ChunkReader]] for chunked encoding
  * [[$curdir.LengthReader]] for messages with a Content-Length header field.
  * [[$curdir.LengthReader]] for messages without a body (GET messages).
  * [[$curdir.ReaderOnly]] for other messages.

  ### Arguments ###

  * _conn_: A file-like object, connected to an HTTP server

  ### Return value ###

  An object containing the following properties:

  * _statusLine_: The first line from the server, without trailing newline.
  * _headers_: An object containing HTTP headers returned from the
    server, converted to lower camel-case.
  * _stream_: A file-like object which can be used to read the message
    body.


 */

function(conn) {

  var sl=conn.readline();

  // this happens when connection is closed since last use
  if (sl=="") {
    return;//throw new Error("http: No status line");
  }
  
  var ret={
    statusLine: sl.replace(/\r\n$/,""),
    headers: $parent.mime.readHeaders(conn)
  };

  if (ret.headers.transferEncoding=="chunked")
    ret.stream=new ChunkReader(conn);
  else if (ret.headers.contentLength)
    ret.stream=new LengthReader(conn, ret.headers.contentLength);
  else if (sl.substr(0,3)=="GET")
    ret.stream=new LengthReader(conn,0);
  else
    ret.stream=new ReaderOnly(conn);
  
  return ret;
}

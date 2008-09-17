/*
    serveConnection(conn)

When an http client has connected to the stream conn, this function
handles the request, using the ActiveDirectory object hostdir/clientdir as its document root.
The connection is kept open for a while to serve subsequent requests from the
same client.

    */

// this is a server object

function (conn) {
  var global=function(){return this;}();

  if (!('listDir' in this.options))
    this.options.listDir = true;
  if (!('keepAlive' in this.options))
    this.options.keepAlive = 3000;

//clib.puts("newconn");
  for (;;) {
    var hasOwnProperty=Object.prototype.hasOwnProperty;
      
    var msg=readMessage(conn);
    if (conn.eof()) return;

    if (this.activeRequests==0) {
      this.clientdir.$checkdates();
      global.$checkdates();
      $parent.js['export'].global.$checkdates();
    }

    this.activeRequests++;
      
    var cx={};
    cx.requestHeaders=msg.headers;
    stdin=msg.stream;
    cx.remoteAddress=conn.remoteAddress();
      
    var statusParts=msg.statusLine.split(" ");
      
    var url="http://";
    if (cx.requestHeaders.host)
      url+=cx.requestHeaders.host;
      
    url+=statusParts.slice(1,statusParts.length-1).join(" ");
    cx.requestURL=url;
    //clib.puts(url);
      
    var parts=decodeURI(url);
      
    var extension=parts.path.match(/\.([^.]*)$/);
    if (extension)
      extension=extension[1];
      
    var filename=parts.path.replace("/",JSEXT_config.sep);
    try {
      filename=$parent.normalize(filename);
      if (filename.substr(0,(JSEXT_config.sep+JSEXT_config.pardir).length)==JSEXT_config.sep+JSEXT_config.pardir)
	filename=JSEXT_config.sep;
    } catch(x) {
      filename=JSEXT_config.sep;
    }
      
    var isdir=false;
    var exists=$parent.exists(this.hostdir.$path+filename);
    if (exists)
      isdir=$parent.isdir(this.hostdir.$path+filename);
      
    if (isdir && $parent.exists(this.hostdir.$path+filename+JSEXT_config.sep+"index.html")) {
      filename=filename+JSEXT_config.sep+"index.html";
      isdir=false;
    }
      
    cx.method=statusParts[0];
    var r_ok=$parent.access(this.hostdir.$path+filename, "r");

    var sendcookies=Cookie.bake.call(cx);

    function writeHeaders(stream) {
      stdin.close();
      if (cx.responseHeaders.location)
	cx.responseLine="302 Found";
      stream.write("HTTP/1.1 "+cx.responseLine+"\r\n");
      sendcookies(stream);
      $parent.mime.writeHeaders(stream, cx.responseHeaders);
    }

    cx.responseLine="200 OK";
    cx.responseHeaders={contentType: 'text/html',
			transferEncoding: 'chunked'};
    stdout=new ChunkWriter(new FlashWriter(conn, writeHeaders));

    cx.hostdir=this.hostdir;
    cx.clientdir=this.clientdir;
    cx.filename=filename;
    cx.activeRequests=this.activeRequests;

    if (this.activeRequests==1) {
	this.hostdir.$checkdates();
	this.clientdir.$checkdates();
	$parent.js['export'].global.$checkdates();
    }

    if (!exists) {
      notFound.call(cx);
    } else if (isdir) {
      if (this.options.listDir)
	listDir.call(cx);
      else
	forbidden.call(cx);
    } else if (!r_ok) {
      forbidden.call(cx);
    } else if (extension=="jsx" && (parts.qryString!==undefined || cx.method!="GET")) {
      execScript.call(cx, this.activeRequests==1);
    } else if (extension=="js" && (parts.qryString!==undefined || cx.method!="GET")) {
      exportScript.call(cx, this.activeRequests==1);
    } else {
      sendFile.call(cx);
    }
      
    stdin.close();
    stdout.close(); // ends chunks
    conn.write("\r\n"); // sends trailer
      
    conn.flush();
    this.activeRequests--;

    //clib.puts("...");
    if (this.shutDown || cx.requestHeaders.connection=="close")
      return; // shutdown
    if ($parent.select([conn],undefined,undefined,this.options.keepAlive)==0)
      return; // timeout
    if (conn.eof())
      return;
  }
    
}


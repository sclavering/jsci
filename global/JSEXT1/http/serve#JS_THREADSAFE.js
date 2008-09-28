/*
    serve(server)

Called by [[$curdir.Server]] to wait for incoming connections.
Each new connection spawns a new thread which calls
[[$curdir.serveConnection]].

Returns no value.

    */

// ''this'' is a Server object

function(server) {
  server.hostdir=new $parent.ActiveDirectory(server.path);
  server.hostdir.$url="/";
  server.clientdir=new $parent.ActiveDirectory(server.path, $parent.js['export'].handlers, $parent.js['export'].platform);
  server.clientdir.$url="/";

  var nthreadLock=new $parent.CondVar;
  var nthreads=0;
  this.activeRequests=0;

  $parent.Thread.unshare('stdin');
  $parent.Thread.unshare('stdout');
  $parent.Thread.unshare('stderr');
       
  while (!server.shutDown) {
    try {

      var conn=server.socket.accept();
      if (server.options.onconnect)
        server.options.onconnect.call(server, conn);

      nthreads++;
      if (nthreads==1 && server.options.onidle)
	server.options.onidle.call(server);

      new $parent.Thread(newthread, conn);
    
    } catch(x) {
      // Probably a shutdown
    }
  }

  nthreadLock.wait(function(){return nthreads==0;});

  $parent.Thread.share('stdin');
  $parent.Thread.share('stdout');
  $parent.Thread.share('stderr');

  server.socket.close();

  function newthread(conn) {

    try {
      serveConnection.call(server,conn);
    } catch(x) {
	clib.puts(x.toSource());
      // presumably, the connection was closed by client, the request had errors or something
    }

    conn.close();

    nthreadLock.update(function(){--nthreads;});
  }

}


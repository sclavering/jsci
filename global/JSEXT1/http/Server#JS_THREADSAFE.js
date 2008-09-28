/*

    srv = new Server([host [, port [, path [, options]]]])

An HTTP server accepts http requests on the given host and port.

### Arguments ###

All arguments are optional.

* _host_: Address of interface on which to listen. _undefined_ is any interface.
* _port_: Port on which to listen. Default is 80, but only root is allowed to listen on ports below 1024.
* _path_: Path to document root. Default is the current working directory.
* _options_: An object containing options

### Options ###

* _listDir_: Boolean specifying if directory listing is allowed. Default is true.
* _keepAlive_: Number of milliseconds to keep a connection up after each request has finished. Default is 3000.
* options belonging to [[$parent.tcp.Listen]].

If the process receives a SIGINT signal, all pending requests are finished and the
function returns. If another SIGINT is received before pending requests are finished,
the default action is taken, which is usually to terminate the program.

Each connection spawns a new thread, which lives as long as the connection.

Mime types are determined based on filename extensions, as defined in [[$parent.mime]].
JavaScript files are passed as source code when the method is GET and there is
no query string (i.e. no question mark in the URL). Otherwise, JavaScript files
are executed and the output sent to the client.

The server runs in its own thread, and can be shut down by calling [[$curdir.Server.prototype.close]].

    */

(function(){

  var s=function(host, port, path, options) {
  
    options = options || {};

    this.port=port || 80;
    this.path=path || $parent.getcwd();
    this.socket=new $parent.tcp.Listen(host, port, options);
    this.path=path;
    this.options=options;

    this.thread=new $parent.Thread(serve, this);
    
  }

  s.prototype={

  /*

            server.close([wait = false])

        Shut down the server. If _wait_ is true, the function will not
        return before the server has been shut down, finishing all current connections.

         */

  close:function(wait) {
    this.shutDown=true;

    if (this.socket) {
      try {
	var fp=$parent.tcp.connect(this.host, this.port);
	fp.close();
      } catch(x) {}
    }

    if (wait)
      this.thread.join();
  }
  }

  return s;

})()

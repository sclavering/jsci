/*
    fcgi([path] [, backlog])

Initializes the fcgi library and awaits incoming requests.
This function never returns. This function is
called by [[$curdir.shell]] when the environment variable
_JSEXT\_FCGI_ is set.

If threads are enabled, each incoming request spawns a new thread.
Otherwise, only one request can be handled at any one time.
Each request calls
[[$curdir.fcgi.serveRequest]] to handle the request.
That function is passed a [[$curdir.fcgi.Request]] object.

The [[stdin]], [[stdout]], [[stderr]] and [[environment]]
objects are _unshared_ (see [[$curdir.Thread.unshare]]),
so that each thread can use
them to communicate with the correct http client.

### Arguments ###

* _path_: The file name of a named socket used for communication with the web server
* _backlog_: The maximum number of waiting requests

### NOTE ###

The JavaScript interpreter does not exit between requests, so any globally accessible objects that are created during the processing of one request are accessible to the next. In particular, modules that are loaded during the processing of one request remain loaded for the lifetime of the fcgi server, and are not automatically reloaded until the fcgi server is restarted. This is good for performance, but bad for development. Therefore, CGI is preferrable during development of a web site.

### NOTE 2 ###

A CGI application may call [[$curdir.exit]] to exit at any point. In fcgi, that would end the fcgi server in an incorrect way. Therefore, [[fcgi.server]] replaces
[[$curdir.exit]] with a function that throws an exception. That exception is caught by _fcgi_, which passes the exit code on to the web server through a fastcgi message. While the
original [[$curdir.exit]] will bypass any _try_ ... _catch_ constructions which surround your call to [[$curdir.exit]], the fcgi version can not.


*/

function(path, backlog) {

  if (0!=arguments.callee.lib.FCGX_IsCGI())
    throw new Error("fcgi: isCgi");
//  if (0!=arguments.callee.lib.FCGX_Init())
//    throw new Error("fcgi: Init");

  var old_exit=$curdir.exit;
  $curdir.exit=function(status) {	// replace exit function
    if (status==undefined)
    status=0;
    arguments.callee.status=status;
    throw arguments.callee;
  }

  var listen=new arguments.callee.Listen(path, backlog);

  try {
    if (JSEXT_config.JS_THREADSAFE) {
      Thread.unshare('stdin');
      Thread.unshare('stdout');
      Thread.unshare('stderr');
      Thread.unshare('environment');
      
      for(;;) {
	new Thread(arguments.callee.serveRequest,listen.accept()).detach();
      }
      
      Thread.share('stdin');
      Thread.share('stdout');
      Thread.share('stderr');
    } else {
      for(;;) {
	arguments.callee.serveRequest(listen.accept());
      }
    }
  } catch(x) {
    // server shutdown?
  }

  listen.close();
  $curdir.exit=old_exit;
}



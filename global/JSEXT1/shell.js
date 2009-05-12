/*
  This function is called automatically during the initialization of
  JSEXT.

  ### Invoked from a web server ###

  When JSEXT is started as an fcgi server, the web server should
  set an environment variable named JSEXT_FCGI and pass no
  arguments to JSEXT. *shell* will then call [[$curdir.fcgi]] to
  start the fcgi server.

  ### Invoked with no arguments ###

  If JSEXT is started without arguments, an interactive session
  is started by calling [[$curdir.interactive]].
*/

function() {
  if(environment.JSEXT_FCGI) {
    return JSEXT1.fcgi();
  }

  if(environment.GATEWAY_INTERFACE) {
    new JSEXT1.CGI();
    return;
  }

  if(!arguments.length) return JSEXT1.interactive();

  return 1;
}

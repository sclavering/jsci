/*
         serveRequest(req)

     Handles an HTTP request. Sets up stdin, stdout and environment
     and calls cgi.run[[$parent.cgi.serverRequest]].

     ### Arguments ###

     A [[$curdir.Request]] object

    */

(function(){

  var activeRequests=0;

  return function (req) {

    activeRequests++;

    stdin=req['in'];
    stderr=req['err'];
    stdout=req['out'];
    environment=req['env'];
    
    $parent.cgi.run(activeRequests==1 && JSEXT_config.JS_THREADSAFE);
    req.close();
    activeRequests--;
  }
})()
 

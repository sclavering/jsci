/*
         serveRequest(req)

     Handles an HTTP request. Sets up stdin, stdout and environment
     and calls new CGI[[$parent.CGI]].

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
    
    new $parent.CGI(activeRequests==1 && JSEXT_config.JS_THREADSAFE);
    req.close();
    activeRequests--;
  }
})()
 

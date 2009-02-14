/*
         serveRequest(req)

     Handles an HTTP request. Sets up stdin, stdout and environment
     and calls new CGI[[$parent.CGI]].

     ### Arguments ###

     A [[$curdir.Request]] object

    */

function(req) {
    stdin=req['in'];
    stderr=req['err'];
    stdout=req['out'];
    environment=req['env'];
    new $parent.CGI();
    req.close();
}

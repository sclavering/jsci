function(stayalive) {

  var server=new $parent.http.Server(undefined, 5001, arguments.callee.$path, {reuseAddr: true});
  if (stayalive)
    return server;
  var wsdl=toXML($parent.read("http://localhost:5001/exp.jsx?"));
  var serv=importWSDL(wsdl);
  var ret=serv.myService.func1([1,2,3],2,3)
  server.close(true);
  return ret;
  
}

/*

Function which connects to tcp ports. Use this function to
implement clients.

    stream = tcp.connect(host, port)

Returns a [[$parent.File]] object connected to the given host and port.

*/

function(host, port) {
  res=[null];
  var err;

  clib.signal(clib.SIGPIPE, clib.SIG_IGN);
  host = host || "localhost";

  err=clib.getaddrinfo(String(host), String(port), null, res);
  if (err)
    throw new Error("connect: "+clib.gai_strerror(err).string());
  
  try {
    var ai=res[0].$;
    
    var sockfd=clib.socket(ai.ai_family, ai.ai_socktype, ai.ai_protocol);
    
    if (sockfd==-1)
      throw new Error($parent.error("connect"));
    
    if (clib.connect(sockfd, ai.ai_addr, ai.ai_addrlen))
      throw new Error($parent.error("connect"));
    
  } catch(x) {
    
    throw(x);
    
  } finally {
    
    clib.freeaddrinfo(res[0]);
    
  }
  
  var fp=clib.fdopen(sockfd, "r+");
  if (!fp) {
    throw new Error($parent.error("accept"));
  }
  fp.finalize=clib.fclose;
  ret=new $parent.File(fp);
  return ret;
}

(function(curdir) {
/*
    Object used for listening to tcp ports. Use this to
    implement servers.

        srv = new tcp.Listen(host, port[, options]) 

    * _host_: may be undefined, in which case connections to all
      interfaces are accepted.
    * _port_: Number
    * _options_:
      * _reuseAddr_: Boolean. Release port as soon as listening stops.
      * _backlog_: integer. Maximum length of connection queue.
*/


  var c=function(host, port, options) {

    var obj;

    if (this.constructor != arguments.callee) {
      obj={};
      obj.__proto__=arguments.callee.prototype;
    } else
      obj=this;

    clib.signal(clib.SIGPIPE, clib.SIG_IGN);
    options = options || {};
    if (!('backlog' in options))
      options.backlog=8;

    res=[null];

    if (!host) {

      var ai={
	ai_family: clib.PF_INET,
	ai_socktype: clib.SOCK_STREAM,
	ai_protocol: 0,
	ai_addr: Pointer(clib['struct sockaddr_in'],
	  {
	    sa_family_t: clib.AF_INET,
	    sin_port: clib.htons(Number(port)),
	    sin_addr: {
	      s_addr: clib.INADDR_ANY
	    }
	  }),
	ai_addrlen: clib['struct sockaddr_in'].sizeof
      };

    } else {
      var err=clib.getaddrinfo(String(host), port?String(port):null, null, res);
      if (err)
        throw new Error("Listen: "+clib.gai_strerror(err).string());
      var ai=res[0].$;
    }
    
    try {

      var sockfd=clib.socket(ai.ai_family, ai.ai_socktype, ai.ai_protocol);
      
      if (sockfd==-1)
	throw new Error($parent.error("Listen"));
      
      if (options.reuseAddr) {
	if (clib.setsockopt(sockfd, clib.SOL_SOCKET, clib.SO_REUSEADDR, Type.pointer(Type.int), [1], Type.int.sizeof))
	  throw new Error($parent.error("Listen"));
      }

      if (clib.bind(sockfd, ai.ai_addr, ai.ai_addrlen))
        throw new Error($parent.error("Listen")); 
      
    } catch(x) {

      throw(x);

    } finally {

      if (host)
        clib.freeaddrinfo(res[0]);
      
    }

    if (clib.listen(sockfd, options.backlog)==-1)
      throw(new Error($parent.error("Listen")));

    obj.sockfd=sockfd;
    obj.name="<Tcp "+host+":"+port+">";
    obj.ai=ai;

    return obj;
  }

  c.prototype={

   /*
         stream = srv.accept([bufsize])
     
     Blocks until a connection is available. The returned
     object is a [[$parent.File]].

    */

    accept: function(bufsize) {
      var ai=this.ai;
      var addr=Pointer.malloc(ai.ai_addrlen);
      var retsize=[ai.ai_addrlen];
      var sock=clib.accept(this.sockfd, addr, retsize);
      if (sock==-1)
        throw(new Error($parent.error("accept")));
      var fp=clib.fdopen(sock, "a+");
      if (!fp) {
	throw new Error($parent.error("accept"));
      }
      fp.finalize=clib.fclose;
      ret=new $parent.File(fp, bufsize);
      ret.name=this.name;

      ret.remoteAddress=function() {
	var taddr=addr.cast(clib['struct sockaddr']);
	var len=clib.INET6_ADDRSTRLEN;
	var buf=Pointer.malloc(len);
	switch(taddr.member(0,'sa_family').$) {
	  case clib.AF_INET:
	    taddr=taddr.cast(clib['struct sockaddr_in']);
	    if (clib.inet_ntop(clib.AF_INET, taddr.member(0,'sin_addr'), buf, len)==null)
	      throw new Error($parent.error("inet_ntop"));
	    break;
	  case clib.AF_INET6:
	    taddr=taddr.cast(clib['struct sockaddr_in6']);
	    if (clib.inet_ntop(clib.AF_INET6, taddr.member(0,'sin6_addr'), buf, len)==null)
	      throw new Error($parent.error("inet_ntop"));
	    break;
	}
	return buf.string();
      }

      return ret;
    },

    closed: false,

   /*

         num = srv.fileno()
     
     Returns the operating system's file descriptor
     for the listening socket.

    */

    fileno: function() {
      return this.sockfd;
    },

   /*

         srv.close()
     
     Stops listening to the port.

    */
    close: function() {
      if (!this.closed) {
	clib.close(this.sockfd);
	this.closed=true;
      }
    }

  };

  return c;

})(this)
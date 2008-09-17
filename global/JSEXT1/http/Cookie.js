/*
  An object which represents a cookie.

      new Cookie(name, str [, options])

  Options are:

  * _expires_: A [[Date]]. If none is given, cookie lasts as long as user session.
  * _path_: String
  * _domain_: String

 */

(function(){

  function Cookie(str, options) {
    options = options || {};
    
    this.str = str;
    this.options = options;
  }

  Cookie.test=function() {
    x=new Cookie("mycookie","hei",{expires: new Date()});
    print(x,"\n");
    print(x.toHeader(),"\n");

    var obj={requestHeaders:{cookie: "a=b; c=d; e=f"}};
    var s=Cookie.bake.call(obj);
    print(obj.cookie,"\n")
    obj.cookie='RMID=732423sdfs73242; expires=Fri, 31-Dec-2010 23:59:59 GMT; path=/; domain=.example.net';
    obj.cookie='ppkcookie2=yet another test; expires=Fri, 27 Jul 2001 02:47:11 UTC; path=/'
    print(obj.cookie,"\n")
    s(stdout);
  }

  /*
        func = bake()

    Defines a getter and setter for the _cookie_ property
    of the passed 'this' object, which should also
    contain a _requestHeaders_ property.

    The semantics is the same as the document.cookie property in
    a browser.

    Returns a function which, when called with one file-like
    argument, writes the cookie headers to it.
   */

  Cookie.bake=function() {
    var self=this;
    this.requestCookies={};
    var responseCookies={};
    var cookies={};
    responseCookies.__proto__=cookies;

    var p=this.requestHeaders.cookie;
    if (p) {
      p=p.split("; ");
      for (var i=0; i<p.length; i++) {
	var d=p[i].split("=");
	cookies[d[0]]=new Cookie(d[1]);
	this.requestCookies[d[0]]=d[1];
      }
    }

    this.__defineSetter__("cookie", setCookie);
    this.__defineGetter__("cookie", getCookie);

    return writeCookie;

    function setCookie(val) {
      if (typeof val === "string")
	val = Cookie.fromHeader(val);
      responseCookies[val.name]=val;
    }

    function getCookie() {
      var ret="";
      for (var i in responseCookies) {
	if (ret) ret+="; ";
	ret+=i+"="+responseCookies[i];
      }
      return ret;
    }

    function tooLate(val) {
      throw new Error("Too late to set cookie after writing to page");
    }

    function writeCookie(stream) {
      for (var i in responseCookies) {
	if (responseCookies.hasOwnProperty(i)) {
	  stream.write(responseCookies[i].toHeader(i)+"\r\n");
	}
      }
      self.__defineSetter__("cookie", tooLate);
    }
  }

  /*
        cookie = fromHeader(header)

    Construct a new cookie from a header line sent from client to server.

    Returns the new cookie.
   */

  Cookie.fromHeader = function(header) {
    var attrib=header.split(/; +/);
    var options={};
    var name;
    var value;
    for (var i=0; i<attrib.length; i++) {
      var p=attrib[i].split("=");
      if (i==0) {
	name=p[0];
	value=p[1];
      } else {
	if (p[0]=="expires")
	  p[1]=new Date(Date.parse(p[1]));
	options[p[0]]=p[1];
      }
    }

    var cookie = new Cookie(value, options);
    cookie.name=name;

    return cookie;
  }

  Cookie.prototype={
    toString: function() {
      return this.str;
    },

    /*
          string = cookie.toHeader(name)

      Return a header line which sends this cookie from server to client.
     */

    toHeader: function(name) {
      var ret="Set-Cookie: "+name+"="+this.str;
      if (this.options.expires && typeof this.options.expires.toUTCString=="function")
	ret+="; expires="+this.options.expires.toUTCString();
      if (this.options.path)
	ret+="; path="+this.options.path;
      if (this.options.domain)
	ret+="; domain="+this.options.domain;
      
      return ret;
    }
  }

  return Cookie;

})()

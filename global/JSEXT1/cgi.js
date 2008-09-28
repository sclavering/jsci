/*
         cgi([refresh=false])

     Interprets the environment variables given in
     _environment_ according to the CGI/1.1 specification:
     <http://hoohoo.ncsa.uiuc.edu/cgi/env.html>.

     The CGI standard treats some HTTP headers differently
     from others. That distinction is reversed by the _cgi_
     function. All headers are placed in the
     requestHeaders property of the created context object.

     This function uses the web server's url-to-path translation.
     The behaviour of _cgi_ depends on the filename
     extension, query string and the HTTP method:

     <table>
       <tr><td colspan='2' rowspan='3'> </td><th colspan='3'>HTTP method</th></tr>
       <tr><th colspan='2'>GET</th><th rowspan='2'>POST</th></tr>
       <tr><th>without query string</th><th>with query string</th></tr>
       <tr><th rowspan='3'>Filename extension</th><th>.js</th>
         <td>Send file</td><td colspan='2'>Include dependencies</td></tr>
       <tr><th>.jsx</th>
         <td>Send file</td><td colspan='2'>Execute file</td></tr>
       <tr><th>other</th>
         <td colspan='3'>Send file</td></tr>
     </table>

     JSEXT is usually not invoked for other files than _.js_ or _.jsx_,
     but if it is, it will simply send the files, setting the
     content-type according to the filename extension and the
     mime types defined in [[$curdir.mime.type]].

     JavaScript files, both _.js_ and _.jsx_ have mime type
     "application/javascript".

     ### Arguments ###

     * _refresh_: Boolean. If true, _cgi_ will attempt to reload
       ActiveDirectory objects that have been changed since they
       were last loaded.

     ### Context object ###
    
     When _cgi_ calls a _.jsx_ function, it passes a _context object_
     as _this_, which contains the following properties:

     * _requestHeaders_: An object containing the following properties:
       * _contentType_
       * _contentLength_
       * all other HTTP headers passed from the HTTP server
     * _responseHeaders_: An object containing the following property:
       * _contentType_: "text/html"
     * _remoteAddress_
     * _method_: "GET" or "POST"
     * _requestURL_

     For details about the execution context of _.jsx_ files,
     see [[How to write a JSEXT webpage]].

     For details about the inclusion of dependencies of _.js_ files,
     see [[Running JSEXT programs in a browser]].

    */

(function() {

var hostDirCache={};
var clientDirCache={};
 
return function (refresh) {

  
  var sendcookies;

  function writeHeaders(stream) {
    stdin.close();
    if (cx.responseHeaders.location)
      cx.responseLine="302 Found";
    if (cx.responseLine)
      stream.write("Status: "+cx.responseLine+"\r\n");
    sendcookies(stream);
    mime.writeHeaders(stream, cx.responseHeaders);
    delete cx.responseHeaders;
  }
  stdout=new http.FlashWriter(stdout, writeHeaders);
  
  try {

  var cx={};
  var headers={};
  for (var i in environment) {
    if (i.substr(0,5)=="HTTP_")
      headers[i.substr(5).
	      toLowerCase().
	      replace(/_./g,
		      function(a) { return a.toUpperCase().substr(1); }
		      )]=environment[i];
  }

  if (environment.CONTENT_TYPE)
    headers.contentType=environment.CONTENT_TYPE;
  if (environment.CONTENT_LENGTH)
    headers.contentLength=environment.CONTENT_LENGTH;
  cx.requestHeaders=headers;
  cx.remoteAddress=environment.REMOTE_ADDR;
  cx.method=environment.REQUEST_METHOD;
  
  var url="http://";
  if (cx.requestHeaders.host)
    url+=cx.requestHeaders.host;
  url+=environment.REQUEST_URI;
  
  cx.requestURL = url;
    
  // Set a default content type.
  cx.responseHeaders={contentType: 'text/html'};


  sendcookies=http.Cookie.bake.call(cx);

  var filename=environment.PATH_TRANSLATED || environment.SCRIPT_FILENAME;
  var path=$curdir.path(filename);
  var onlyFilename=$curdir.filename(filename);
  var urlpath=environment.PATH_INFO || environment.SCRIPT_NAME;
  var pathParts=path.split(JSEXT_config.sep);
  var urlParts=cx.requestURL.split("/");
  urlParts.pop();

  var i;
  for (i=1; i<=urlParts.length; i++)
    if (pathParts[pathParts.length-i] != urlParts[urlParts.length-i])
      break;

  var root=pathParts.slice(0,pathParts.length-i+1).join(JSEXT_config.sep);
  var rooturl=urlParts.slice(0,urlParts.length-i+1).join("/");
  if (rooturl!="/")
    rooturl+="/";

  pathParts.push(onlyFilename);
  var relFilename=JSEXT_config.sep+pathParts.slice(pathParts.length-i).join(JSEXT_config.sep);

  var extension=relFilename.match(/\.([^.]*)$/);
  if (extension)
    extension=extension[1];

  if (refresh) {
    var global=(function(){return this;})();
    hostDirCache={};
    clientDirCache={};
    global.$checkdates();
    js['export'].global.$checkdates();
  }

  var hostdir=hostDirCache[rooturl];
  if (!hostdir) {
    hostdir={};
    ActiveDirectory.call(hostdir, root);
    hostDirCache[rooturl]=hostdir;
  }

  var clientdir=clientDirCache[rooturl];
  if (!clientdir) {
    clientdir={};
    ActiveDirectory.call(clientdir, root, js['export'].handlers, js['export'].platform);
    clientDirCache[rooturl]=clientdir;
    var relroot = $curdir.path(urlpath.replace(/\//g, JSEXT_config.sep));
    relroot=relroot.split(JSEXT_config.sep);
    relroot=relroot.slice(0,relroot.length-i+1).join("/");
    if (relroot!="/")
      relroot+="/";

    clientdir.$url=relroot;
  }

  cx.clientdir=clientdir;
  cx.hostdir=hostdir;
  cx.filename="/"+pathParts.slice(pathParts.length-i).join(JSEXT_config.sep);
  //  cx.responseLine="200 OK";
  
  // 2008-09-28-22-26 steve: entirely removed the dispatching based on file extension.
  http.execScript.call(cx, refresh);

  } catch(x) {
    print(' <br/>\n');
    if (x.fileName && x.lineNumber)
      print('Line '+x.lineNumber+' in '+x.fileName+':');
    print(x+' <br/>\n');
    if (x.stack) {
      var stack=x.stack.split('\n');
      if (stack.length>6)
	print(stack.slice(1,stack.length-5).join(' <br/>\n')+' <br/>\n');
    }
  }

  stdout.close();
}

})()




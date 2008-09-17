/*
  I can not for the best of me remember what this function does,
  but it looks long and clever, so I'll keep it for a while.

  It is used by activecache.
 */

function(path, url, options) {

  var mirror=arguments.callee;

  print("MIRROR ",path," ",url,"\n");
  options=options || {};
  options.refreshInterval=options.refreshInterval || 1000*3600*24*7;

  var parts=decodeURI(url);
  if (parts.qryString!==undefined)
    return;

  if (parts.path.substr(-1)=="/")
    mirrorDir();
  else
    mirrorFile();

  function mirrorDir() {
    var fn=parts.path.match(/([^\/]*)\/$/)[1];

    if (!$parent.exists(path+JSEXT_config.sep+fn))
      $parent.mkdir(path+JSEXT_config.sep+fn);

    if (expired(fn+JSEXT_config.sep+"index.html")) {
      refreshFile(fn+JSEXT_config.sep+"index.html");
    }

    var dirdoc=$parent.read(path+JSEXT_config.sep+fn+JSEXT_config.sep+"index.html");
    var dir=$parent.html.listdir(dirdoc);
    print(dir,"\n");
    for (var i in dir) {
      mirror(path+JSEXT_config.sep+fn, url+dir[i], options);
    }
  }

  function mirrorFile() {
    print("mirror file "+parts.path+"\n");
    var fn=parts.path.match(/([^\/]*)$/)[1];
    if (expired(fn)) {
      refreshFile(fn);
    }
  }

  function expired(fn) {
    print("expired "+fn+"\n");
    if (!$parent.exists(path+JSEXT_config.sep+fn+".expires"))
      return true;
    var expires=$parent.read(path+JSEXT_config.sep+fn+".expires");
    return Date.parse(expires) < new Date();
  }

  function refreshFile(filename) {
    print("refresh "+filename+"\n");
    var stat=$parent.stat(path+JSEXT_config.sep+filename);
    if (stat) {
      var file=get(url,
	  {'If-Modified-Since': stat.mtime},
	  {followRedirect: false});
    } else {
      var file=get(url);
    }
    
    if (file.statusLine[9]!='3') {
      $parent.write(path+JSEXT_config.sep+filename, file.document);
      if (file.headers['Last-Modified']) {
	var buf=Pointer(clib['struct utimbuf']);
	buf.$={actime: new Date().valueOf()/1000,
	       modtime: Date.parse(file.headers['Last-Modified'].replace(/-/g," "))/1000
	};
	clib.utime(path+JSEXT_config.sep+filename, buf);
      }
    }
    
    if (file.headers.Expires)
      var expires=file.headers.Expires.replace(/-/g," ");
    else
      var expires=String(new Date(new Date().valueOf() + options.refreshInterval));

    $parent.write(path+JSEXT_config.sep+filename+".expires", expires);
  }
  
}

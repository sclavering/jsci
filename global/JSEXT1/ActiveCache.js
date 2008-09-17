/*
ActiveCache
---

Same as [[$curdir.ActiveWeb]], but with local caching.

Constructor
---


    new ActiveCache( path, url, [handlers, [options]] )

* _path_: [[String]] Name of directory where cache lives
* _url_: [[String]] Url to the source (a http directory listing)
* _handlers_: [[Object]] contains one key per file ending and a function for each file ending. The function
will be called with only the full file name (and not path) as the only argument. The _this_ object
will be the invoking ActiveDirectory object. The current working directory will be set to the directory
containing the file.
* _options_: [[Object]] containing options like refreshInterval in milliseconds.

Files and directories which are ignored by ActiveDirectory, i.e. +include, +tinyCdb etc, are mirrored
by http.mirror.

Properties
---


* _$name_: [[String]] which contains the name of the property, except for root object, which by default has no name.
* _$parent_: [[Object]] which contains the parent ActiveDirectory
* _$path_: [[String]] which contains the path of the directory
* _$url_: [[String]] which contains the url of the directory

*/


function( path, url, handlers, options ) {

  options=options || {};
  options.refreshInterval=options.refreshInterval || 1000*3600*24*7;

  url=url.replace(/\/$/,"");

  if (!exists(path))
    mkdir(path);

  // Update directory listing

  try {
    var dir=load(path+JSEXT_config.sep+"0-ActiveCache.dir");
    if (Date.parse(read(path+JSEXT_config.sep+"0-ActiveCache.expires")) < new Date()) {
      throw("dir listing expired");
    }
  } catch(x) {
    var stat=stat(path+JSEXT_config.sep+"0-ActiveCache.dir");
    if (stat) {
      var htmldir=http.get(url+"/",
	                   {'If-Modified-Since': stat.mtime},
	                   {followRedirect: false});
    } else {
      var htmldir=http.get(url+"/");
    }
    if (htmldir.document!="") {
      var dir=html.listdir(htmldir.document);
      write(path+JSEXT_config.sep+"0-ActiveCache.dir", dir.toSource());
    }

    var expires=htmldir.headers.Expires;
    if (!expires)
      expires=String(new Date(new Date().valueOf() + options.refreshInterval));

    write(path+JSEXT_config.sep+"0-ActiveCache.expires", expires.replace(/-/g," "));
  }

  var filename;
  var subdirs=[];
  var self=this;
  var getters={};
  var ActiveCache=arguments.callee;
  var hasOwnProperty=Object.prototype.hasOwnProperty;

  this.$path=path;
  this.$url=url;

  var dirobj={};
  for (var i in dir) {
    var filename=dir[i];
    dirobj[filename]=true;
  }

  for (var i in dir) {
    var filename=dir[i];
    //	  clib.puts(" "+filename);
    if (filename[0].match(/[^ -@]/)) {
      var dotpos=filename.lastIndexOf('.');
      if (dotpos !== -1) {
	var propname=filename.substr(0, dotpos);
	var extension=filename.substr(dotpos+1);
	if (handlers[extension] && !hasOwnProperty.call(this,propname) && propname!="valueOf") {
	  getters[propname]=getGetter(propname, extension);
	  //	  delete this[propname];
	  this.__defineGetter__(propname, getters[propname]);
	  this.__defineSetter__(propname, getDefaultSetter(propname));
	} else if (handlers[extension] && propname=="prototype") {
	  getGetter(propname, extension).call(this);
	}
      } else if (filename.substr(-1)=='/') {
	subdirs.push(filename.substr(0,filename.length-1));
      }
    } else {
      http.mirror(path, url + "/" + filename, options);
    }
  }

  for (var i in subdirs) {
      var filename=subdirs[i];
    if (hasOwnProperty.call(this,filename) && !getters[filename]) {
      // When making a function, the 'prototype' property will be automatically created.
      // If there is also a 'prototype' directory, then read it right away - not possible
      // to defer. The test above works because the 'prototype' property will have been
      // defined (hasOwnProperty), but without a getter.

      if (typeof(this)=="function" && filename=="prototype") {
	var newpath=path + JSEXT_config.sep + filename;
	if (!exists(newpath))
	  mkdir(newpath);
	var newurl=url + "/" + filename;
	ActiveCache.call(this[filename], newpath, newurl, handlers, options);
      }

      this[filename].$name=filename;
      this[filename].$parent=this;

    } else {
      this.__defineGetter__(filename, getSubdirGetter(filename, hasOwnProperty.call(getters, filename) && getters[filename]));
      this.__defineSetter__(filename, getDefaultSetter(filename));
    }
  }

  function getDefaultSetter(propname) {

    return function(value) {
      delete self[propname];
      this[propname]=value;
    }

  }

  function getSubdirGetter(filename, oldgetter) {
    return function() {
      if (oldgetter) {
	var val=oldgetter.call(this);
      } else {
	delete this[filename];
	var val=(this[filename]={});
      }

      var newpath=path + JSEXT_config.sep + filename;
      var newurl=url + "/" + filename;
      ActiveCache.call(val, newpath, newurl, handlers, options);
      val.$name=filename;
      this[filename].$parent=this;

      return val;
    }
  }

  function getGetter(propname, extension) {
    return function() {
      //      clib.puts("getting "+propname+" path="+path);
      var olddir;

      if (!exists(propname+".expires") ||
	  Date.parse(read(propname+".expires")) < new Date()) {
	refresh(propname);
      }

      var val = handlers[extension].call(self, propname, "."+extension);

      delete self[propname];
      this[propname] = val;

      return val;
    }
  }

  function refresh(propname) {
    // Called when cwd is in directory

    var expires=Infinity;

    for (var i in handlers) {
      if (dirobj[propname+"."+i])
	refreshFile(propname+"."+i);
    }

    if (expires < Infinity)
      write(propname+".expires", new Date(expires));

    function refreshFile(filename) {

      var stat=stat(filename);
      if (stat) {
	var file=http.get(url+"/"+filename,
	                  {'If-Modified-Since': stat.mtime},
	                  {followRedirect: false});
      } else {
	var file=http.get(url+"/"+filename);
      }

      if (file.statusLine[9]!='3') {
	write(filename, file.document);
	if (file.headers['Last-Modified']) {
	  var buf=Pointer(clib['struct utimbuf']);
	  buf.$={actime: new Date().valueOf()/1000,
		 modtime: Date.parse(file.headers['Last-Modified'].replace(/-/g," "))/1000
                };
	  clib.utime(filename, buf);
	}
      }
      
      if (file.headers.Expires)
	expires=Math.min(expires, Date.parse(file.headers.Expires.replace(/-/g," ")));
      else
	expires=Math.min(expires, new Date(new Date().valueOf() + options.refreshInterval));


    }
  }

}

/*
ActiveWeb
---

Provides access to pages on a web server the same way
[[$curdir.ActiveDirectory]] provides access to files in a
file system. If you want to download pages explicitly,
use [[$curdir.http.get]] instead.


Constructor
---


    new ActiveWeb( url, [handlers] )

* _url_: [[String]] Url to directory to mirror
* _handlers_: [[Object]] contains one key per file ending and a function for each file ending. The function
will be called with only the full file name (and not path) as the only argument. The _this_ object
will be the invoking ActiveDirectory object. The current working directory will be set to the directory
containing the file.

Properties
---


* _$name_: [[String]] which contains the name of the property, except for root object, which by default has no name.
* _$parent_: [[Object]] which contains the parent ActiveDirectory
* _$url_: [[String]] which contains the url of the directory

*/


function( url, handlers, platform ) {

  url=url.replace(/\/$/,"");

  var dir=html.listdir(http.get(url+"/").document);
  var filename;
  var subdirs=[];
  var self=this;
  var getters=this.$getters || {};
  var ActiveWeb=arguments.callee;

  handlers = handlers || activate;
  platform = platform || JSEXT_config;

  var hasOwnProperty=Object.prototype.hasOwnProperty;

  this.$url=url;
  this.$getters=getters;
  this.$curdir=this;

  var parts;

  for (var i in dir)
    if (hasOwnProperty.call(dir,i)) {
    var filename=dir[i];
    parts=filename.match(/^(([^ -@][^#\.]*)(#([^\.]*))?)(\.(.*))?/);
    if (parts && (!parts[4] || platform[parts[4]])) { // correct platform
      if (parts[5]) {
	var propname=parts[2];
	var extension=parts[6];
	if (handlers[extension] && !hasOwnProperty.call(this,propname) && propname!="valueOf") {
	  getters[propname]=getGetter(propname, parts[1], extension);
	  //	  delete this[propname];
	  if (this.__defineGetter__) {
	    this.__defineGetter__(propname, getters[propname]);
	    this.__defineSetter__(propname, getDefaultSetter(propname));
	  }
	} else if (handlers[extension] && propname=="prototype") {
	  getGetter(propname, parts[1], extension).call(this);
	}
      } else if (filename.substr(-1)=='/') {
	subdirs.push(parts);
      }
    }
  }

  for (var i in subdirs)
    if (hasOwnProperty.call(subdirs,i)) {
    var parts=subdirs[i];
    if (hasOwnProperty.call(this,parts[2]) && !getters[parts[2]]) {
      // When making a function, the 'prototype' property will be automatically created.
      // If there is also a 'prototype' directory, then read it right away - not possible
      // to defer. The test above works because the 'prototype' property will have been
      // defined (hasOwnProperty), but without a getter.

      if (typeof(this)=="function" && filename=="prototype") {
	var newurl=url + "/" + parts[0];
	ActiveWeb.call(this[parts[1]], newurl, handlers, platform);
      }

      this[parts[2]].$curdir=this[parts[2]];
      this[parts[2]].$name=parts[2];
      this[parts[2]].$parent=this;

    } else {
      getters[parts[2]]=getSubdirGetter(parts[2], parts[0], hasOwnProperty.call(getters, parts[2]) && getters[parts[2]]);

      if (this.__defineGetter__) {
	this.__defineGetter__(parts[2], getters[parts[2]]);
	this.__defineSetter__(parts[2], getDefaultSetter(parts[0]));
      }
    }
  }

  function getDefaultSetter(propname) {

    return function(value) {
      delete self[propname];
      this[propname]=value;
    }

  }

  function getSubdirGetter(propname, filename, oldgetter) {
    return function() {
      if (oldgetter) {
	var val=oldgetter.call(this);
      } else {
	try { delete this[propname]; } catch(x) {}
	var val=(this[propname]={});
      }

      var newurl=url + "/" + filename;
      ActiveWeb.call(val, newurl, handlers, platform);
      val.$name=propname;
      this[propname].$parent=this;

      return val;
    }
  }

  function getGetter(propname, filename, extension) {
    return function() {
      //      alert("getting "+propname+" filename="+filename+" url="+url);
      var olddir;

      var val = handlers[extension].call(self, filename, "."+extension);
      //      alert("val="+val);
      try { delete self[propname]; } catch(x) {}
      this[propname] = val;

      return val;
    }
  }

  function manualGetter(propname) {
    if (!hasOwnProperty.call(this,propname))
      getters[propname]();
  }
}

/*

This object maps files and directories to properties and objects. Say you have this directory structure:

    mydir/
    mydir/some_text.txt
    mydir/a_function.js
    mydir/a_function/
    mydir/a_function/prototype/
    mydir/a_function/prototype/a_method.js
    mydir/a_directory/
    mydir/a_directory/more_text.txt

Calling

    var x=new ActiveDirectory('mydir');

Will give you the following object structure

    x                                // is an object
    x.some_text                      // is a string: The contents of some_text.txt
    x.a_function                     // is a function: The one stored in a_function.js
    x.a_function.prototype           // is an object
    x.a_function.prototype.a_method  // is a function: The one stored in a_method.js
    x.a_directory                    // is an object
    x.a_directory.more_text          // is a string: The contents of more_text.txt

Note that the various files and subdirectories are only loaded, listed and evaluated if and when the corresponding
JavaScript properties are
accessed for the first time; so-called lazy evaluation. Therefore, there is no performance or
memory penalty associated with activating a large directory structure, as long as you only
use a small part of it.

In addition to the user-defined properties, each object is automagically equipped with the
properties $name, $path and (except for the root object) $parent.

The action taken to convert each file into a javascript value is determined by the file name extension.
By default, .js files are evaluated, .txt files are read, etc.
Or you may supply your own set of handler functions. They should be passed as an optional second
argument to ActiveDirectory. That argument should be an object, where each property name corresponds to
a filename extension (minus the dot), and each property is a function which handles the conversion from file
to value. For example, the default txt handler looks like this:

    function(name,extension) {
      var file = new JSEXT1.File(this.$path + '/' + name + extension, "r");
      var ret=file.read();
      file.close();
      return ret;
    }

Subdirectories are handled by ActiveDirectory, and become new ActiveDirectory objects with the
same handler functions as the parent directory.

Files and directories which start with any ASCII character less than A are ignored.
So if you name a directory something like _0-subdir_,
it and its contents will be hidden from the JavaScript realm.
Directories whose names include a dot are ignored. Files with unknown extensions are ignored.
When a directory contains more than one file with the same base name but different extensions,
any one of their handlers is called. Which one is undefined.


Constructor
---

new ActiveDirectory( path, [handlers] )

### Arguments ###

* _path_: [[String]] Name of directory to activate
* _handlers_: [[Object]] contains one function property per file extension. The function
will be called with two arguments: The bare file name (minus extension and path), and extension (including the dot) as the second argument. The _this_ object
will be the invoking ActiveDirectory object. The function may always find the path to the file it is loading in _this.$path_. The property names
should correspond to file name extensions without the dot.

Properties of ActiveDirectory objects
---

* _$name_: [[String]] which contains the name of the property, except for root object, which by default has no name.
* _$curdir_: [[Object]] which contains the current ActiveDirectory
* _$parent_: [[Object]] which contains the parent ActiveDirectory
* _$path_: [[String]] which contains the path of the directory
* _$checkdates_: [[Function]] which can be called to update any properties
  whose files have been updated on disk.

Calling as a function
---


If ActiveDirectory is called as a function rather than as a constructor, it will populate the _this_ object with the
properties stored in the given directory.

*/

function(path, handlers) {
  var dir=$curdir.dir(path);
  var filename;
  var subdirs=[];
  var self=this;

	// Always use 'self' because 'this' may be down the prototype chain somewhere
	// when the ActiveDirectory object is the prototype of another object

  var ActiveDirectory=arguments.callee;

  handlers = handlers || JSEXT1.activate;

  var hasOwnProperty=Object.prototype.hasOwnProperty;

  self.$path=path;
  self.$curdir=self;
  self.$checkdates=checkdates;

  if (!hasOwnProperty.call(self,'$getters')) {
    self.$getters={};
    self.$loaded={};
    self.$dirs={};
    self.$mydate=$curdir.stat(path).mtime;
  }

  var parts;

  for (var i in dir)
    if (hasOwnProperty.call(dir,i)) {
      var filename=dir[i];
      parts=filename.match(/^(([^ -@][^#\.]*)(#([^\.]*))?)(\.(.*))?/);
      
      if (parts && !parts[4]) {
	if (parts[5]) {
	  var propname=parts[2];
	  var extension=parts[6];
	  if (handlers[extension] && !hasOwnProperty.call(self,propname) && propname!="valueOf") {
	    self.$getters[propname]=getGetter(propname, parts[1], extension);
	    //	  delete self[propname];
	    self.__defineGetter__(propname, self.$getters[propname]);
	    self.__defineSetter__(propname, getDefaultSetter(propname));
	  } else if (handlers[extension] && propname=="prototype") {
	    self.prototype = handlers[extension].call(self, parts[1], '.'+extension);
	    //	  getGetter(propname, parts[1], extension).call(self);
	  }
	} else if ($curdir.isdir(path + '/' + filename)) {
	  subdirs.push(parts);
	}
      }
    }

  for (var i in subdirs)
    if (hasOwnProperty.call(subdirs,i)) {
    var parts=subdirs[i];
    if (hasOwnProperty.call(self,parts[2]) && !self.$getters[parts[2]]) {
      // When making a function, the 'prototype' property will be automatically created.
      // If there is also a 'prototype' directory, then read it right away - not possible
      // to defer. The test above works because the 'prototype' property will have been
      // defined (hasOwnProperty), but without a getter.

      // However, when ActiveDirectory is called to refresh an existing directory,
      // it must be prevented from doing self.

	if (!self.__lookupGetter__(parts[2])) {

	if (typeof(self)=="function" && parts[0]=="prototype") {
	  var val=self[parts[1]];
		/*
             if (hasOwnProperty.call(self,"$glocal")) {
                           val.__proto__=self.$glocal;
               val.$glocal=self.$glocal;
             }
                   */
	  var newpath = path + '/' + parts[0];
	  ActiveDirectory.call(val, newpath, handlers);
	}

	self[parts[2]].$curdir=self[parts[2]];
	self[parts[2]].$name=parts[2];
	self[parts[2]].$parent=self;
      }

    } else {
      self.$getters[parts[2]]=getSubdirGetter(parts[2], parts[0], hasOwnProperty.call(self.$getters, parts[2]) && self.$getters[parts[2]]);
      self.__defineGetter__(parts[2], self.$getters[parts[2]]);
      self.__defineSetter__(parts[2], getDefaultSetter(parts[2]));
    }
  }

  /*
     if (hasOwnProperty.call(self,"glocal")) {
                   print("have glocal!\n");
         self.glocal.__proto__=self.__proto__;
         self.__proto__=self.glocal;
         self.$glocal=self.glocal;
     }  
     */

  function getSubdirGetter(propname, filename, oldgetter) {
    return function() {
      if (oldgetter) {
	var val=oldgetter.call(self);
      } else {
	delete self[propname];
	var val=(self[propname]={});
	//					val.__proto__=self;
      }

      self.$dirs[propname]=val;

      /*
         if (hasOwnProperty.call(self,"$glocal")) {
           val.__proto__=self.$glocal;
           val.$glocal=self.$glocal;
         }
         */

      var newpath = path + '/' + filename;
      ActiveDirectory.call(val, newpath, handlers);
      val.$name=propname;
      self[propname].$parent=self;
      return val;
    }
  }

  function getDefaultSetter(propname) {

    return function(value) {
      delete self[propname];
      self[propname]=value;
    }

  }

  function getGetter(propname, filename, extension) {
    return function() {
      //	      clib.puts("getting "+propname);
      var olddir;

      delete self[propname];
      self[propname]=undefined;
      try {
	var val = handlers[extension].call(self, filename, "."+extension);
	var check = new String(propname);
	check.mtime = $curdir.stat(self.$path + '/' + filename + "." + extension).mtime;
	self.$loaded[filename+"."+extension]=check;
      } catch (x) {
	delete self[propname];
	self.__defineGetter__(propname, arguments.callee);
	self.__defineSetter__(propname, getDefaultSetter(propname));
	throw(x);
      }

	//	      clib.puts("got "+propname);

      if (self[propname]===undefined)
	self[propname] = val;

      return val;
    }
  }

  function checkdates() {
    //    clib.puts("checkdates "+self.$path);
    if ($curdir.stat(path).mtime>self.$mydate) {
      //	clib.puts("dirupdate");
      ActiveDirectory.call(self, path, handlers);
    }

    for (var file in self.$loaded) {
      var prop=self.$loaded[file];
      var propname=String(prop);
      var stat = $curdir.stat(self.$path + '/' + file);
      if (!stat || stat.mtime>prop.mtime) {
//		clib.puts("refresh "+file);
	delete self[propname];
	delete self.$loaded[file];
 	delete self.$dirs[propname];
	if (stat) {
	  //	  	  clib.puts(String(self.$getters[propname]));
	  self.__defineGetter__(propname, self.$getters[propname]);
	  self.__defineSetter__(propname, getDefaultSetter(propname));
	}
      }
    }

    for (var dir in self.$dirs) {
      self.$dirs[dir].$checkdates();
    }
  }

}


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
*/
(function() {


function ActiveDirectory(path, handlers) {
  var self=this;

  // Always use 'self' because 'this' may be down the prototype chain somewhere when the ActiveDirectory object is the prototype of another object

  var hasOwnProperty=Object.prototype.hasOwnProperty;

  self.$path=path;
  self.$curdir=self;
  self.$handlers = handlers = handlers || JSEXT1.activate;

  if(!hasOwnProperty.call(self, '$getters')) self.$getters = {};

  var subdirs=[];

  var dir = $curdir.dir(path);
  for(var i in dir) {
    if(!hasOwnProperty.call(dir, i)) continue;

    var filename = dir[i];
    var parts = filename.match(/^([^ -@][^.]*)(?:\.(.*))?/);

    if(!parts) continue;

    var propname = parts[1], extension = parts[2];
    if(extension) {
      if(handlers[extension] && !hasOwnProperty.call(self, propname) && propname != "valueOf") {
        self.$getters[propname] = getGetter(self, propname, extension);
        self.__defineGetter__(propname, self.$getters[propname]);
        self.__defineSetter__(propname, getDefaultSetter(self, propname));
      } else if(handlers[extension] && propname == "prototype") {
        self.prototype = handlers[extension].call(self, propname, '.' + extension);
      }
    } else if($curdir.isdir(path + '/' + filename)) {
      subdirs.push(parts);
    }
  }


  for(var i in subdirs) {
    if(!hasOwnProperty.call(subdirs, i)) continue;

    var filename = subdirs[i][0], propname = subdirs[i][1];
    if(hasOwnProperty.call(self, propname) && !self.$getters[propname]) {
      // When making a function, the 'prototype' property will be automatically created.
      // If there is also a 'prototype' directory, then read it right away - not possible
      // to defer. The test above works because the 'prototype' property will have been
      // defined (hasOwnProperty), but without a getter.

      // However, when ActiveDirectory is called to refresh an existing directory,
      // it must be prevented from doing self.

      if(!self.__lookupGetter__(propname)) {
        if(typeof(self) == "function" && filename == "prototype") {
          var val = self[propname];
          var newpath = path + '/' + filename;
          ActiveDirectory.call(val, newpath, handlers);
        }
        self[propname].$curdir = self[propname];
        self[propname].$name = propname;
        self[propname].$parent = self;
      }

    } else {
      self.$getters[propname] = getSubdirGetter(self, propname, filename, hasOwnProperty.call(self.$getters, propname) && self.$getters[propname]);
      self.__defineGetter__(propname, self.$getters[propname]);
      self.__defineSetter__(propname, getDefaultSetter(self, propname));
    }
  }
}


function getSubdirGetter(self, propname, filename, oldgetter) {
    return function() {
      if(oldgetter) {
        var val = oldgetter.call(self);
      } else {
        delete self[propname];
        var val = self[propname] = {};
      }

      var newpath = self.$path + '/' + filename;
      ActiveDirectory.call(val, newpath, self.$handlers);
      val.$name=propname;
      self[propname].$parent=self;
      return val;
    }
}


function getDefaultSetter(self, propname) {
    return function(value) {
      delete self[propname];
      self[propname] = value;
    }
}


function getGetter(self, propname, extension) {
    return function() {
      var olddir;

      delete self[propname];
      self[propname] = undefined;
      try {
        var val = self.$handlers[extension].call(self, propname, "." + extension);
      } catch (x) {
        delete self[propname];
        self.__defineGetter__(propname, arguments.callee);
        self.__defineSetter__(propname, getDefaultSetter(self, propname));
        throw(x);
      }

      if(self[propname] === undefined) self[propname] = val;

      return val;
    }
}


return ActiveDirectory;

})()

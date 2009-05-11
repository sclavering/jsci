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
  self.$handlers = handlers = handlers || ActiveDirectory.handlers;

  if(!hasOwnProperty.call(self, '$getters')) self.$getters = {};

  var subdirs=[];

  var dir = JSEXT1.dir(path);
  for(var i in dir) {
    if(!hasOwnProperty.call(dir, i)) continue;

    var parts = dir[i].match(/^([^ -@][^.]*)(?:\.(.*))?/);
    if(!parts) continue;

    var propname = parts[1], extension = parts[2];
    if(extension) {
      if(handlers[extension] && !hasOwnProperty.call(self, propname) && propname != "valueOf") {
        self.$getters[propname] = make_getter(self, propname, extension);
        self.__defineGetter__(propname, self.$getters[propname]);
        self.__defineSetter__(propname, make_setter(self, propname));
      } else if(handlers[extension] && propname == "prototype") {
        self.prototype = handlers[extension].call(self, propname, '.' + extension);
      }
    } else if(JSEXT1.isdir(path + '/' + propname)) {
      subdirs.push(propname);
    }
  }


  for(var i in subdirs) {
    if(!hasOwnProperty.call(subdirs, i)) continue;

    var propname = subdirs[i];
    if(hasOwnProperty.call(self, propname) && !self.$getters[propname]) {
      // When making a function, the 'prototype' property will be automatically created.
      // If there is also a 'prototype' directory, then read it right away - not possible
      // to defer. The test above works because the 'prototype' property will have been
      // defined (hasOwnProperty), but without a getter.

      // However, when ActiveDirectory is called to refresh an existing directory,
      // it must be prevented from doing self.

      if(!self.__lookupGetter__(propname)) {
        if(typeof self == "function" && propname == "prototype") {
          var val = self[propname];
          var newpath = path + '/' + propname;
          ActiveDirectory.call(val, newpath, handlers);
        }
        self[propname].$curdir = self[propname];
        self[propname].$name = propname;
        self[propname].$parent = self;
      }

    } else {
      self.$getters[propname] = make_subdir_getter(self, propname, hasOwnProperty.call(self.$getters, propname) && self.$getters[propname]);
      self.__defineGetter__(propname, self.$getters[propname]);
      self.__defineSetter__(propname, make_setter(self, propname));
    }
  }
}


function make_subdir_getter(self, propname, oldgetter) {
    return function() {
      if(oldgetter) {
        var val = oldgetter.call(self);
      } else {
        delete self[propname];
        var val = self[propname] = {};
      }

      var newpath = self.$path + '/' + propname;
      ActiveDirectory.call(val, newpath, self.$handlers);
      val.$name=propname;
      self[propname].$parent=self;
      return val;
    }
}


function make_setter(self, propname) {
    return function(value) {
      delete self[propname];
      self[propname] = value;
    }
}


function make_getter(self, propname, extension) {
    return function() {
      var olddir;

      delete self[propname];
      self[propname] = undefined;
      try {
        var val = self.$handlers[extension].call(self, propname, "." + extension);
      } catch (x) {
        delete self[propname];
        self.__defineGetter__(propname, arguments.callee);
        self.__defineSetter__(propname, make_setter(self, propname));
        throw(x);
      }

      if(self[propname] === undefined) self[propname] = val;

      return val;
    }
}


// A mapping from file extensions to handler functions to load and process those files.
// It's occasionally used outside of this module
ActiveDirectory.handlers = {
  js: handle_script,

  txt: handle_text,
  html: handle_text,

  c: handle_native,
  h: handle_native,
  so: handle_native,
  jswrapper: handle_native,
}


/*
JavaScript files are interpreted with the current directory's ActiveDirectory object on top of the scope chain.
*/
function handle_script(name, extension) {
  return load.call(this, this.$path + '/' + name + extension);
}


function handle_text(name, extension) {
  var file = new JSEXT1.File(this.$path + '/' + name + extension, "r");
  var ret = file.read();
  file.close();
  return ret;
}


/*
This handler handles .h, .c, .jswrapper, and .so files.

.h and .c files are source files. .jswrapper and .so files are generated automatically from .h and .c files if necessary.

If a .c file exists, it is compiled as a dynamic library and used as the default .so file when parsing the .h file.

If no .h file exists (but a .c file), then function declarations are extracted from the .c file.

When wrapping a system library there is just a .h file and no .c file, and only a .jswrapper file will be generated.

The .jswrapper file is a javascript file returning an obejct containing the low-level type-marshalling wrappers for C code (using Type, Pointer, etc.).  Each property
*/
function handle_native(name, extension) {
  // 1. Find timestamps of all files in group

  var timestamp={};

  //clib.puts("cget "+this.$path+"/"+name);
  for each(var ext in ['h', 'c', 'jswrapper', 'so']) {
    var stat = JSEXT1.stat(this.$path + '/' + name + '.' + ext);
    timestamp[ext] = stat && stat.mtime;
  }

  // 2. Build .so if possible and necessary

  var dlobj;

  if(timestamp.c && timestamp.c > timestamp.so) {
    //clib.puts("ccget "+name);
    JSEXT1.chdirLock(this.$path);
    JSEXT1.C.compile(name+'.c');
    var stat = JSEXT1.stat(name + '.so');
    timestamp.so = stat && stat.mtime;
    JSEXT1.chdirUnlock();
  }

  if(timestamp.so) {
    JSEXT1.chdirLock(this.$path);
    dlobj = Dl('./' + name + '.so');
    JSEXT1.chdirUnlock();
  }

  // 3. If it is a jsapi module, just load it.

  if (dlobj && dlobj.symbolExists('JSX_init')) {
    var ret = dlobj['function']('JSX_init')();
    return ret;
  }

  // 4. Build .jswrapper file if possible and necessary

  if(timestamp.h && timestamp.h > timestamp.jswrapper) {
    JSEXT1.chdirLock(this.$path);
    var ps = new JSEXT1.Progress;
    ps.status("Parsing h file");
    var fragment = JSEXT1.C.fragment(JSEXT1.read(name + '.h'), dlobj);
    ps.status("Writing .jswrapper file");
    var wrapperfile = new JSEXT1.File('./' + name + '.jswrapper', 'w');
    wrapperfile.write(JSEXT1.C.jswrapper(fragment));
    wrapperfile.close();
    ps.close();
    timestamp.jswrapper = JSEXT1.stat('./' + name + '.jswrapper');
    JSEXT1.chdirUnlock();
  }

  // 5. Load .jswrapper file if possible

  if(timestamp.jswrapper) {
    JSEXT1.chdirLock(this.$path);
    var ret = load.call(this, this.$path + '/' + name + '.jswrapper');
    for(var i = 0; ret['dl ' + i]; i++) { /* pass */ }
    JSEXT1.chdirUnlock();
    return ret.hasOwnProperty("main") ? ret.main : ret;
  }
}


return ActiveDirectory;

})()

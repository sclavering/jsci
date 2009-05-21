/*
ActiveDirectory

This object lazily maps files and directories to properties on objects.

Usage:

    ActiveDirectory.get(path)               // returns a new or cached object
    ActiveDirectory.get(path, some_object)  // adds properties to some_object (and also returns it)

(For backwards compatibility, |new ActiveDirectory(path)| and |ActiveDirectory.call(some_object, path)| may also be used.)

Say you have this directory structure:

    mydir/
    mydir/some_text.txt
    mydir/a_function.js
    mydir/a_function/
    mydir/a_function/prototype/
    mydir/a_function/prototype/a_method.js
    mydir/a_directory/
    mydir/a_directory/more_text.txt

Calling ...

    var x = new ActiveDirectory('mydir');

... will give you the following object structure:

    x                                // is an object
    x.some_text                      // is a string: The contents of some_text.txt
    x.a_function                     // is a function: The one stored in a_function.js
    x.a_function.prototype           // is an object
    x.a_function.prototype.a_method  // is a function: The one stored in a_method.js
    x.a_directory                    // is an object
    x.a_directory.more_text          // is a string: The contents of more_text.txt

Files and subdirectories are loaded lazily, i.e. the first time the corresponding property is accessed.

In addition to the user-defined properties, each object is automagically equipped with the
properties $name, $path and (except for the root object) $parent.

The action taken to convert each file into a javascript value is determined by the file name extension:
  .js files are evaluated
  .txt files are read
  .c .h and .so files are compiled if necessary, and a .jswrapper file created that does the necessary Pointer and Type magic to expose to JavaScript all the C functions, types, and variable, as well as any constants created via #define
  .jswrapper files are evaluated like .js files

Subdirectories are handled by ActiveDirectory, and become new ActiveDirectory objects.

Files and directories which start with any ASCII character less than A are ignored (e.g. this includes anything starting with a digit), as are subfolders whose name contains a dot, and files with unknown extensions.

If there are both e.g. a "foo.js" and a "foo.txt", one or other will be chosen abitrarily.

ActiveDirectory maintains an internal cache of all previously-created ActiveDirectory objects (keyed by filesystem path), and will always return a cached object if available rather than creating a new one.  You should thus be careful about mutating ActiveDirectory instances (since the alterations will be shared).  The cache is unlimited in size, and never expired.

Properties of ActiveDirectory instances:

* $name: [[String]] which contains the name of the property, except for root object, which by default has no name.
* $curdir: [[Object]] which contains the current ActiveDirectory
* $parent: [[Object]] which contains the parent ActiveDirectory
* $path: [[String]] which contains the path of the directory
*/
(function() {


const hasOwnProperty = Object.prototype.hasOwnProperty;


const cached_instances = ActiveDirectory.cache = {
}; // mapping from paths to already-created ActiveDirectory instances


// ActiveDirectory used to be a constructor, and we retain backwards compatibility.
function ActiveDirectory(path) {
  ActiveDirectory.get(path, this);
}


ActiveDirectory.get = function get(path, obj) {
  // "self" is misnamed for historical reasons
  var self = obj || {};

  if(!obj && cached_instances[path]) return cached_instances[path];

  self.$path=path;
  self.$curdir=self;

  if(!hasOwnProperty.call(self, '$getters')) self.$getters = {};

  var subdirs=[];

  var dir = JSEXT1.os.dir(path);
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
    } else if(JSEXT1.os.isdir(path + '/' + propname)) {
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
      if(!self.__lookupGetter__(propname)) {
        if(typeof self == "function" && propname == "prototype") {
          var val = self[propname];
          var newpath = path + '/' + propname;
          ActiveDirectory.get(newpath, val);
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

  if(!obj) cached_instances[path] = self;

  return self;
}


function make_subdir_getter(self, propname, oldgetter) {
    return function() {
      var val, newpath = self.$path + '/' + propname;

      if(oldgetter) {
        val = oldgetter.call(self);
        ActiveDirectory.get(newpath, val);
      } else {
        delete self[propname];
        val = self[propname] = ActiveDirectory.get(newpath);
      }

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
      delete self[propname];
      self[propname] = undefined;
      try {
        var val = handlers[extension].call(self, propname, "." + extension);
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
const handlers = ActiveDirectory.handlers = {
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
  const chdir_stack = [];

  // 1. Find timestamps of all files in group

  var timestamp={};

  //clib.puts("cget "+this.$path+"/"+name);
  for each(var ext in ['h', 'c', 'jswrapper', 'so']) {
    var stat = JSEXT1.os.stat(this.$path + '/' + name + '.' + ext);
    timestamp[ext] = stat && stat.mtime;
  }

  // 2. Build .so if possible and necessary

  var dlobj;

  if(timestamp.c && timestamp.c > timestamp.so) {
    pushd(chdir_stack, this.$path);
    JSEXT1.C.compile(name+'.c');
    var stat = JSEXT1.os.stat(name + '.so');
    timestamp.so = stat && stat.mtime;
    popd(chdir_stack);
  }

  if(timestamp.so) {
    pushd(chdir_stack, this.$path);
    dlobj = Dl('./' + name + '.so');
    popd(chdir_stack);
  }

  // 3. If it is a jsapi module, just load it.

  if (dlobj && dlobj.symbolExists('JSX_init')) {
    var ret = dlobj['function']('JSX_init')();
    return ret;
  }

  // 4. Build .jswrapper file if possible and necessary

  if(timestamp.h && timestamp.h > timestamp.jswrapper) {
    pushd(chdir_stack, this.$path);
    var ps = new JSEXT1.Progress;
    ps.status("Parsing h file");
    var fragment = JSEXT1.C.fragment(name + '.h', dlobj);
    ps.status("Writing .jswrapper file");
    var wrapperfile = new JSEXT1.File('./' + name + '.jswrapper', 'w');
    wrapperfile.write(JSEXT1.C.jswrapper(fragment));
    wrapperfile.close();
    ps.close();
    timestamp.jswrapper = JSEXT1.os.stat('./' + name + '.jswrapper');
    popd(chdir_stack);
  }

  // 5. Load .jswrapper file if possible

  if(timestamp.jswrapper) {
    pushd(chdir_stack, this.$path);
    var ret = load.call(this, this.$path + '/' + name + '.jswrapper');
    for(var i = 0; ret['dl ' + i]; i++) { /* pass */ }
    popd(chdir_stack);
    return ret.hasOwnProperty("main") ? ret.main : ret;
  }
}


function pushd(chdirstack, dir) {
  const cwd = JSEXT1.os.getcwd();
  if(clib.chdir(dir) == -1) throw new Error(os.error("chdirLock"));
  else chdirstack.push(cwd);
}


function popd(chdirstack) {
  clib.chdir(chdirstack.pop());
}


return ActiveDirectory;

})()

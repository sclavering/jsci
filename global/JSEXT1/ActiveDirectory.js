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

Each object also gets a .$path property.

The action taken to convert each file into a javascript value is determined by the file name extension:
  .js files are evaluated
  .txt files are read
  .h files are used to build a .jswrapper file that does the necessary Pointer and Type magic to expose to JavaScript all the C functions, types, and variable, as well as any constants created via #define
  .jswrapper files are evaluated like .js files

Subdirectories are handled by ActiveDirectory, and become new ActiveDirectory objects.

Files and directories which start with any ASCII character less than A are ignored (e.g. this includes anything starting with a digit), as are subfolders whose name contains a dot, and files with unknown extensions.

If there are both e.g. a "foo.js" and a "foo.txt", one or other will be chosen abitrarily.

ActiveDirectory maintains an internal cache of all previously-created ActiveDirectory objects (keyed by filesystem path), and will always return a cached object if available rather than creating a new one.  You should thus be careful about mutating ActiveDirectory instances (since the alterations will be shared).  The cache is unlimited in size, and never expired.

Properties of ActiveDirectory instances:

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
    return self[propname] = handlers[extension].call(self, propname, "." + extension);
  }
}


// A mapping from file extensions to handler functions to load and process those files.
// It's occasionally used outside of this module
const handlers = {
  js: handle_script,

  txt: handle_text,
  html: handle_text,

  h: handle_native,
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
This handler handles .h files, and the .jswrapper files generated from them.

The .jswrapper file is a javascript file returning an obejct containing the low-level type-marshalling wrappers for C code (using Type, Pointer, etc.).
*/
function handle_native(name, extension) {
  const path = this.$path + '/';
  var stat_h = JSEXT1.os.stat(path + name + '.h');
  if(!stat_h) return undefined;
  var stat_w = JSEXT1.os.stat(path + name + '.jswrapper');

  if(!stat_w || stat_h.mtime > stat_w.mtime) {
    JSEXT1.File.write(path + name + '.jswrapper', JSEXT1.wraplib(path + name + '.h'));
  }

  var ret = load.call(this, path + name + '.jswrapper');
  for(var i = 0; ret['dl ' + i]; i++) { /* pass */ }
  return ret;
}


return ActiveDirectory;

})()

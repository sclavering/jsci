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

The action taken to convert each file into a javascript value is determined by the file name extension:
  .js files are evaluated
  .txt files are read
  .h files are used to build a .jswrapper file that does the necessary Pointer and Type magic to expose to JavaScript all the C functions, types, and variable, as well as any constants created via #define
  .jswrapper files are evaluated like .js files

Subdirectories are handled by ActiveDirectory, and become new ActiveDirectory objects.

Files and directories which start with any ASCII character less than A are ignored (e.g. this includes anything starting with a digit), as are subfolders whose name contains a dot, and files with unknown extensions.

If there are both e.g. a "foo.js" and a "foo.txt", one or other will be chosen abitrarily.

ActiveDirectory maintains an internal cache of all previously-created ActiveDirectory objects (keyed by filesystem path), and will always return a cached object if available rather than creating a new one.  You should thus be careful about mutating ActiveDirectory instances (since the alterations will be shared).  The cache is unlimited in size, and never expired.
*/
(function() {


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

  const getters = {};
  const subdirs = [];

  for each(var filename in JSEXT1.os.dir(path)) {
    var parts = filename.match(/^([^ -@][^.]*)((?:\..*)?)$/);
    if(!parts) continue;
    var propname = parts[1], extension = parts[2];
    if(extension) {
      if(!/^\.(?:js|txt|html|h|jswrapper)$/.test(extension)) continue;

      if(!self.hasOwnProperty(propname) && !(propname in Object.prototype)) {
        getters[propname] = make_getter(self, path, propname, extension);
        self.__defineGetter__(propname, getters[propname]);
        self.__defineSetter__(propname, make_setter(self, propname));
      } else if(propname == "prototype") {
        self.prototype = handle(self, path, propname, '.' + extension);
      }
    } else if(JSEXT1.os.isdir(path + '/' + propname)) {
      subdirs.push(propname);
    }
  }

  for each(var propname in subdirs) {
    if(self.hasOwnProperty(propname) && !getters[propname]) {
      // function objects always have a .prototype property, so we can't just install a getter to lazily create it
      if(!self.__lookupGetter__(propname)) {
        if(typeof self == "function" && propname == "prototype") {
          ActiveDirectory.get(path + '/' + propname, self[propname]);
        }
      }

    } else {
      getters[propname] = make_subdir_getter(self, path, propname, getters[propname] || null);
      self.__defineGetter__(propname, getters[propname]);
      self.__defineSetter__(propname, make_setter(self, propname));
    }
  }

  if(!obj) cached_instances[path] = self;

  return self;
}


function make_subdir_getter(self, path, propname, oldgetter) {
    return function() {
      var val, newpath = path + '/' + propname;
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
      return self[propname] = value;
    }
}


function make_getter(self, path, propname, extension) {
  return function() {
    delete self[propname];
    return self[propname] = handle(self, path, propname, extension);
  }
}


function handle(self, path, name, extension) {
  const proppath = path + '/' + name, filename = proppath + extension;
  switch(extension) {
    case ".js":
      return load.call(self, filename);
    case ".txt":
    case ".html":
      return JSEXT1.File.read(filename);
    case ".h":
      if(!os.exists(proppath + '.jswrapper')) JSEXT1.File.write(proppath + '.jswrapper', JSEXT1.wraplib(proppath + '.h'));
      // fall through
    case ".jswrapper":
      var ret = load.call(self, proppath + '.jswrapper');
      // xxx make the wrapper file do this itself
      for(var i = 0; ret['dl ' + i]; i++) { /* pass */ }
      return ret;
  }
  return undefined;
}


return ActiveDirectory;

})()

/*
Module(path)
Module(path, obj)

Our module system, which maps lazily maps file/folder structures into javascript objects.

Given the directory structure:

    foo/
    foo/some_text.txt
    foo/some_value.js
    foo/some_obj/bar.js
    foo/some_obj/bar/baz.js

Calling Module('path/to/foo') will return an object with the lazy properties:

    .some_text          // a string containing the contents of some_text.txt
    .some_value         // whatever javascript value some_value.js ultimately evaluates to
    .some_obj           // an object, which itself has the lazy property...
        .bar            // whatever javascript value bar.js evaluates to, with the addition of the lazy property...
            .baz        // whatever javascript value baz.js evaluates to

By "lazy properties", we mean a getter defined using __defineGetter__, which loads the file (or iterates the subfolder) only when the property is first accessed.

Files with the extensions ".js", ".txt", ".h" and ".jswrapper" create lazy properties:
  .js files are evaluated, potentially with side-effects, and the last value computed is used
  .txt files contents are converted to a javascript string (by padding out each byte to a 16-bit javascript character)
  .jswrapper files are treated like .js files, but are typically generated by JSEXT, and use Type, Pointer and Dl to wrap a C library
  .h files are preprocessed and parsed as C header files, and used to build and write a .jswrapper file, which is then run

Subfolders are handled by Module, and become new Module objects.  Their .__proto__ field is set to the parent Module object, so that the parent's properties are in-scope within the subfolder.  e.g. in the above example, code in baz.js would be able to refer to "bar", and also (via __proto__.__proto__) to "some_obj", "some_text", and "some_value".

Files and folders which start with any ASCII character less than A are ignored (e.g. this includes anything starting with a digit), as are subfolders whose name contains a dot.

If there are both e.g. a "foo.js" and a "foo.txt", one or other will be chosen abitrarily.

Module maintains an internal cache of all previously-created Module objects (keyed by filesystem path), and will always return a cached object if available rather than creating a new one.  You should thus be careful about mutating Module instances (since the alterations will be shared).  The cache is unlimited in size, and never expired.
*/
(function() {


const cached_instances = Module.cache = {
}; // mapping from paths to already-created Module instances


// Module used to be a constructor, and we retain backwards compatibility.
function Module(path, obj) {
  // "self" is misnamed for historical reasons
  let self = obj || {};

  if(!obj && cached_instances[path]) return cached_instances[path];

  const getters = {};
  const subdirs = [];

  for each(let filename in JSEXT1.os.dir(path)) {
    let parts = filename.match(/^([^ -@][^.]*)((?:\..*)?)$/);
    if(!parts) continue;
    let propname = parts[1], extension = parts[2];
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

  for each(let propname in subdirs) {
    if(self.hasOwnProperty(propname) && !getters[propname]) {
      // function objects always have a .prototype property, so we can't just install a getter to lazily create it
      if(!self.__lookupGetter__(propname)) {
        if(typeof self == "function" && propname == "prototype") {
          Module(path + '/' + propname, self[propname]);
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
    let val, newpath = path + '/' + propname;
    if(oldgetter) {
      val = oldgetter.call(self);
      Module(newpath, val);
    } else {
      delete self[propname];
      val = self[propname] = Module(newpath);
    }
    // So code within the subfolder has self's properties available in its scope
    val.__proto__ = self;
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
      return load.call(self, proppath + '.jswrapper');
  }
  return undefined;
}


return Module;

})()
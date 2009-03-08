/*
An object mapping file extensions to handler functions used to load and process files for ActiveDirectory (and occasionally elsewhere).
*/
(function() {

return {
  js: handle_script,

  txt: handle_text,
  html: handle_text,

  c: handle_native,
  h: handle_native,
  so: handle_native,
  pch: handle_native,
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
This handler handles .h, .c, .pch, and .so files.

.h and .c files are source files. .pch and .so files are generated automatically from .h and .c files if necessary.

If a .c file exists, it is compiled as a dynamic library and used as the default .so file when parsing the .h file.

If no .h file exists (but a .c file), then function declarations are extracted from the .c file.

When wrapping a system library there is just a .h file and no .c file, and only a .pch file will be generated.

The .pch file is a Cdb file containing the low-level javascript wrappers for what's in the .so file.
*/
function handle_native(name, extension) {
  // 1. Find timestamps of all files in group

  var timestamp={};

  //clib.puts("cget "+this.$path+"/"+name);
  for each(var ext in ['h', 'c', 'pch', 'so']) {
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

  // 4. Build pch file if possible and necessary

  if (timestamp.h && timestamp.h > timestamp.pch) {
    JSEXT1.chdirLock(this.$path);
    var ps = new JSEXT1.Progress;
    ps.status("Parsing h file");
    var fragment = JSEXT1.C.fragment(JSEXT1.read(name + '.h'), dlobj);
		ps.status("Writing cdb file");
    var pch = new JSEXT1.Cdb(name + '.pch', 'w');
    pch.write(fragment);
    pch.close();
    ps.close();
    timestamp.pch = JSEXT1.stat(name + '.pch');
    JSEXT1.chdirUnlock();
  }

  // 5. Load pch file if possible

  if (timestamp.pch) {
    JSEXT1.chdirLock(this.$path);
    var ret = new JSEXT1.ActiveCdb(name + ".pch");
    var i;
    for(i = 0; ret['dl ' + i]; i++) { /* pass */ }
    JSEXT1.chdirUnlock();
    if(ret.hasOwnProperty("main")) return ret.main;
    return ret;
  }
}

})()

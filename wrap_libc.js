(function() {
  const JSX_HOME = jsxlib.environment.JSX_HOME;
  const path = JSX_HOME + '/global/';
  jsxlib.load.call(this, path + "0-ffi.js");

  clib = {};
  clib.chdir = Dl().pointer('chdir', Type['function'](Type.int, [Type.pointer(Type.char)], false, 'cdecl'));
  clib.puts = Dl().pointer('puts', Type['function'](Type.int, [Type.pointer(Type.char)], false, 'cdecl'));

  // Things we need to get JSEXT1.File's popen() stuff working, which is needed for JSEXT1.wraplib() to run cpp
  const size_t = Type.unsigned_long;
  const FILE = Type['void']; // It's actually a struct, but we're only passing it around, so it's simpler not to care
  clib.feof = Dl().pointer('feof', Type['function'](Type.int, [Type.pointer(FILE)], false, 'cdecl'));
  clib.ferror = Dl().pointer('ferror', Type['function'](Type.int, [Type.pointer(FILE)], false, 'cdecl'));
  clib.fread = Dl().pointer('fread', Type['function'](size_t, [Type.pointer(FILE), size_t, size_t, Type.pointer(FILE)], false, 'cdecl'));
  clib.popen = Dl().pointer('popen', Type['function'](Type.pointer(FILE), [Type.pointer(Type.char), Type.pointer(Type.char)], false, 'cdecl'));
  clib.pclose = Dl().pointer('pclose', Type['function'](Type.int, [Type.pointer(FILE)], false, 'cdecl'));

  JSEXT1 = { $path: path + '/JSEXT1' };
  JSEXT1.File = load.call(JSEXT1, path + '/JSEXT1/File.js');
  JSEXT1.CParser = load.call(JSEXT1, path + '/JSEXT1/CParser.js');
  JSEXT1.wraplib = load.call(JSEXT1, path + '/JSEXT1/wraplib.js');

  clib.puts(JSEXT1.wraplib(path + "/clib.h"));
})()

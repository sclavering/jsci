function(args) {
  this.Type = args.Type;
  this.Pointer = args.Pointer;
  this.Dl = args.Dl;
  this.load = args.load;
  this.environment = args.environment;
  this.jsxcore = args;

  load('0-ffi.js');

  clib = {};
  clib.chdir = Dl().pointer('chdir', Type['function'](Type.int, [{ 'const': true, type: Type.pointer(Type.char) }], false, 'cdecl')).$;
  clib.puts = Dl().pointer('puts', Type['function'](Type.int, [{ 'const': true, type: Type.pointer(Type.char) }], false, 'cdecl')).$;

  // Things we need to get JSEXT1.File's popen() stuff working, which is needed for JSEXT1.C.runcpp():
  const size_t = Type.unsigned_long;
  const FILE = Type['void']; // It's actually a struct, but we're only passing it around, so it's simpler not to care
  clib.feof = Dl().pointer('feof', Type['function'](Type.int, [{ 'const': false, type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.ferror = Dl().pointer('ferror', Type['function'](Type.int, [{ 'const': false, type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.fread = Dl().pointer('fread', Type['function'](size_t, [{ 'const': false, type: Type.pointer(FILE) }, { 'const': false, type: size_t }, { 'const': false, type: size_t }, { 'const': false, type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.popen = Dl().pointer('popen', Type['function'](Type.pointer(FILE), [{ 'const': true, type: Type.pointer(Type.char) }, { 'const': true, type: Type.pointer(Type.char) }], false, 'cdecl')).$;
  clib.pclose = Dl().pointer('pclose', Type['function'](Type.int, [{ 'const': false, type: Type.pointer(FILE) }], false, 'cdecl')).$;

  print = function () {} // JSEXT1.C.parse() sometimes calls this

  const path = args.cwd;
  JSEXT1 = { $path: path + '/JSEXT1' };
  JSEXT1.File = load.call(JSEXT1, path + '/JSEXT1/File.js');
  JSEXT1.C = { $path: path + '/JSEXT1/C', $parent: JSEXT1 };
  JSEXT1.C.parse = load.call(JSEXT1.C, path + '/JSEXT1/C/parse.js');
  JSEXT1.C.fragment = load.call(JSEXT1.C, path + '/JSEXT1/C/fragment.js');
  JSEXT1.C.runcpp = load.call(JSEXT1.C, path + '/JSEXT1/C/runcpp.js');
  JSEXT1.C.jswrapper = load.call(JSEXT1.C, path + '/JSEXT1/C/jswrapper.js');

  const fragment = JSEXT1.C.fragment(path + "/clib.h", Dl());
  const jswrapper = JSEXT1.C.jswrapper(fragment);
  clib.puts(jswrapper);
}

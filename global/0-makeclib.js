function(args) {
  this.Type = args.Type;
  this.Pointer = args.Pointer;
  this.Dl = args.Dl;
  this.load = args.load;
  this.environment = args.environment;

  clib = {};
  clib.chdir = Dl().pointer('chdir', Type['function'](Type.int, [{ 'const': true, name: '__path', type: Type.pointer(Type.char) }], false, 'cdecl')).$;
  clib.puts = Dl().pointer('puts', Type['function'](Type.int, [{ 'const': true, name: '__s', type: Type.pointer(Type.char) }], false, 'cdecl')).$;

  // Things we need to get JSEXT1.File's popen() stuff working, which is needed for JSEXT1.C.runcpp():
  const size_t = Type.unsigned_long;
  const FILE = Type['void']; // It's actually a struct, but we're only passing it around, so it's simpler not to care
  clib.feof = Dl().pointer('feof', Type['function'](Type.int, [{ 'const': false, name: '__stream', type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.ferror = Dl().pointer('ferror', Type['function'](Type.int, [{ 'const': false, name: '__stream', type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.fread = Dl().pointer('fread', Type['function'](size_t, [{ 'const': false, name: '__ptr', type: Type.pointer(FILE) }, { 'const': false, name: '__size', type: size_t }, { 'const': false, name:'__n', type: size_t }, { 'const': false, name: '__stream', type: Type.pointer(FILE) }], false, 'cdecl')).$;
  clib.popen = Dl().pointer('popen', Type['function'](Type.pointer(FILE), [{ 'const': true, name: '__command', type: Type.pointer(Type.char) }, { 'const': true, name: '__modes', type: Type.pointer(Type.char) }], false, 'cdecl')).$;
  clib.pclose = Dl().pointer('pclose', Type['function'](Type.int, [{ 'const': false, name: '__stream', type: Type.pointer(FILE) }], false, 'cdecl')).$;

  print = function () {} // JSEXT1.C.parse() sometimes calls this


  var fd = 1;

  function js(name) {
    return load.call(this,name+'.js');
  }

  clib.chdir("JSEXT1");
  JSEXT1={};
  JSEXT1.Progress=function(){};
  JSEXT1.Progress.prototype.status=function(){};
  JSEXT1.Progress.prototype.close=function(){};

  clib.chdir("C");
  JSEXT1.C = { $path: '.', $parent: JSEXT1 };
  JSEXT1.C.parse=js.call(JSEXT1.C,'parse');
  JSEXT1.C.fragment=js.call(JSEXT1.C,'fragment');
  JSEXT1.C.runcpp=js.call(JSEXT1.C,'runcpp');
  JSEXT1.C.ctoxml=js.call(JSEXT1.C,'ctoxml');
  JSEXT1.C.jswrapper = js.call(JSEXT1.C, 'jswrapper');
  clib.chdir('..');

  JSEXT1.$path = '.';
  JSEXT1.File = js.call(JSEXT1, 'File');
  clib.chdir('..');
//  puts(String(JSEXT1.C.ctoxml(JSEXT1.C.cpp('#include <io.h>'))));
  const fragment = JSEXT1.C.fragment(args.cwd + "/clib.h", Dl());
  const jswrapper = JSEXT1.C.jswrapper(fragment);
  clib.puts(jswrapper);
}

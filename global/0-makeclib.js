function(_dl, cwd) {
  var mods=['Type','Pointer','load','Dl'];
  for(var i in mods) this[mods[i]] = _dl('./' + mods[i] + '.so');
  var ActiveCdb = _dl('./JSEXT1/ActiveCdb.so');

  clib = {};
  clib.chdir = Dl().pointer('chdir', Type['function'](Type.int, [{ 'const': true, name: '__path', type: Type.pointer(Type.char) }], false, 'cdecl')).$;
  clib.puts = Dl().pointer('puts', Type['function'](Type.int, [{ 'const': true, name: '__s', type: Type.pointer(Type.char) }], false, 'cdecl')).$;
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
  JSEXT1.C.cpp=js.call(JSEXT1.C,'cpp');
  JSEXT1.C.ctoxml=js.call(JSEXT1.C,'ctoxml');
  clib.chdir('..');
  JSEXT1.$path = '.';
  JSEXT1.Cdb=js.call(JSEXT1,'Cdb');
  clib.chdir("Cdb");
  JSEXT1.Cdb.prototype=js.call(JSEXT1.Cdb,'prototype');
  clib.chdir('..');
  clib.chdir('..');
//  puts(String(JSEXT1.C.ctoxml(JSEXT1.C.cpp('#include <io.h>'))));
  var fragment = JSEXT1.C.fragment('#include "clib.h"', Dl('./clib.so'));
  var pch=new JSEXT1.Cdb(fd,'w');
  pch.write(fragment);
  pch.close();
}

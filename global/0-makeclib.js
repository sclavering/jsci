function(name, config, _dl, cwd) {

  if (config._WIN32) {
    config.pardir='..';
    config.curdir='.';
    conifg.sep='\\';
    config.dlext='.dll';
    config.pathsep=';';
    var platform="_WIN32";
  } else {
    config.pardir='..';
    config.curdir= '.';
    config.sep='/';
    config.dlext='.so';
    config.pathsep=':';
    var platform="__unix__";
  }

  var mods=['Type','Pointer','load','Dl'];
  for (var i in mods) {
    this[mods[i]]=_dl(config.curdir+config.sep+mods[i]+config.dlext);
  }
  var JSEXT_version="1";
  var ActiveCdb=_dl(config.curdir+config.sep+"JSEXT"+JSEXT_version+config.sep+"ActiveCdb"+config.dlext);

  this.JSEXT_config=config;

  clib={};

  if (config._WIN32) {
    clib.chdir=Dl("msvcr90.dll").pointer('_chdir',Type['function'](Type.int,[{'const':true,name:'__path',type:Type.pointer(Type.char)}],false,'cdecl')).$;
    clib.open=Dl("msvcr90.dll").pointer('_open',Type['function'](Type.int,[{'const':true,name:'__path',type:Type.pointer(Type.char)}, {name:'mode',type:Type.int}],true,'cdecl')).$;
    clib.write=Dl("msvcr90.dll").pointer('_write',Type['function'](Type.int,[{name:'fd',type:Type.int}, {'const':true,name:'buffer',type:Type.pointer(Type['void'])}, {name:'count',type:Type.int}],true,'cdecl')).$;
    clib.lseek=Dl("msvcr90.dll").pointer('_lseek',Type['function'](Type.int,[{name:'fd',type:Type.int}, {name:'offset',type:Type.long}, {name:'origin',type:Type.int}],true,'cdecl')).$;
    clib.O_BINARY=0x8000;
    clib.O_CREAT=0x0100;
    clib.O_RDWR=0x0002;
    clib.O_TRUNC=0x0200;
    clib.S_IREAD=0x0100;
    clib.S_IWRITE=0x0080;
    clib.O_EXCL=0x0400;
    clib.O_WRONLY=0x0001;
    
    var fd=clib.open("clib.pch",clib.O_BINARY | clib.O_CREAT | clib.O_RDWR | clib.O_TRUNC, Type.int, clib.S_IREAD | clib.S_IWRITE);
  } else {
    clib.chdir=Dl().pointer('chdir',Type['function'](Type.int,[{'const':true,name:'__path',type:Type.pointer(Type.char)}],false,'cdecl')).$;
    clib.puts=Dl().pointer('puts',Type['function'](Type.int,[{'const':true,name:'__s',type:Type.pointer(Type.char)}],false,'cdecl')).$;
    var fd=1;
  }

  function js(name) {
    return load.call(this,name+'.js');
  }

  clib.chdir("JSEXT1");
  JSEXT1={};
  JSEXT1.Progress=function(){};
  JSEXT1.Progress.prototype.status=function(){};
  JSEXT1.Progress.prototype.close=function(){};
  clib.chdir("C");
  JSEXT1.C={$path:config.curdir, 
	       $parent:JSEXT1};
  JSEXT1.C.parse=js.call(JSEXT1.C,'parse');
  JSEXT1.C.fragment=js.call(JSEXT1.C,'fragment');
  JSEXT1.C.cpp=js.call(JSEXT1.C,'cpp');
  JSEXT1.C.ctoxml=js.call(JSEXT1.C,'ctoxml');
  clib.chdir(config.pardir);
  JSEXT1.$path=config.curdir;
  JSEXT1.Cdb=js.call(JSEXT1,'Cdb');
  clib.chdir("Cdb");
  JSEXT1.Cdb.prototype=js.call(JSEXT1.Cdb,'prototype');
  clib.chdir(config.pardir);
  clib.chdir(config.pardir);
//  puts(String(JSEXT1.C.ctoxml(JSEXT1.C.cpp('#include <io.h>'))));
  var fragment=JSEXT1.C.fragment('#include "clib#'+platform+'.h"', Dl(config.curdir+config.sep+'clib#'+platform+config.dlext));
  var pch=new JSEXT1.Cdb(fd,'w');
  pch.write(fragment);
  pch.close();
}

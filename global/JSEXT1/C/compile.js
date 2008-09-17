/*
    compile(filename [, filename_out]) 


Compiles a C program _filename_ into a shared object file _filename\_out_.
If _filename\_out_ is omitted, the extension .so or .dll is used.
Any errors from the compiler is thrown as an exception.
Invokes an external compiler, which must be installed
for this to work.

*/

function(filename, out_filename) {
    var dotpos=filename.lastIndexOf('.');
    
    if (!out_filename) {
      if (dotpos===-1)
	out_filename=filename+JSEXT_config.dlext;
      else
	out_filename=filename.substr(0,dotpos)+JSEXT_config.dlext;
    }
    
    if (JSEXT_config._WIN32) {
      var cmd='cl /I '+$path+'\\0-include /DXP_WIN /nologo /Tc '+filename+' '+$path+'\\js32.lib /MD /LD /Zm400 /Fe'+out_filename;
      if (JSEXT_config.JS_THREADSAFE)
	cmd+=" /D JS_THREADSAFE=1";
      cmd+=" /D __i386__=1x";
      clib.puts(cmd);
      var ret=$parent.read(cmd+' |');
      var error=false;
      for each (var line in ret.split("\n")) {
	var flds=line.split(":");
	if (flds.length>1 && flds[1].indexOf('error')!=-1)
	  throw($parent.trim(ret));
      }
    } else if (JSEXT_config.__unix__) {
      var cmd='cc -I '+$path+'/0-include -DXP_UNIX -fPIC -x c '+filename+' -shared -o '+out_filename;
      if (JSEXT_config.JS_THREADSAFE)
	cmd+=" -DJS_THREADSAFE=1";
      var ret=$parent.read(cmd+' 2>&1 |');
      var error=false;
      for each (var line in ret.split("\n")) {
	if (line.indexOf('error:')!=-1)
	  throw($parent.trim(ret));
      }
    }
}



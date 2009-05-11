/*
compile(filename [, filename_out])

Compiles a C program _filename_ into a shared object file _filename\_out_.
If _filename\_out_ is omitted, the extension .so is appended.
Any errors from the compiler is thrown as an exception.
Invokes an external compiler, which must be installed for this to work.
*/
function(filename, out_filename) {
  const dotpos = filename.lastIndexOf('.');
  if(!out_filename) out_filename = (dotpos === -1 ? filename : filename.substr(0, dotpos)) + '.so';
  const cmd = 'cc -I ' + $path + '/0-include -DXP_UNIX -fPIC -x c ' + filename + ' -shared -o ' + out_filename;
  const ret = JSEXT1.File.read(cmd + ' 2>&1 |');
  if(/error:/.test(ret)) throw new Error(ret.replace(/^\s+|\s+$/g, ""));
}

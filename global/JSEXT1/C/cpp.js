
/*
string = cpp(code, [include_path])

Runs a string through a C preprocessor.

### Arguments ###

* _code_: A string containing a C program
* _include\_path_: An array containing strings with paths that should be searched when resolving #include directives.

### Return value ###

A string.
*/

(function(){
  const libcpp = Dl($path + '/libcpp.so');

  const cpp = libcpp.pointer('cpp', Type['function'](Type.pointer(Type.char), [
      { 'const': false, name: 'C', type: Type.pointer(Type.char) },
      { 'const': false, name: 'errormsg', type: Type.pointer(Type.pointer(Type.char)) },
      { 'const': false, name: 'include_path', type: Type.pointer(Type.pointer(Type.char)) }
    ], false, 'cdecl')).$;

  const free = libcpp.pointer('cpp_free', Type['function'](Type['void'], [
      { 'const': false, name: 'ptr', type: Type.pointer(Type['void']) }
    ], false, 'cdecl')).$;

  return function(code, include_path) {
    include_path = include_path || [];
    for(var i = 0; i < include_path.length; i++) include_path[i] = String(include_path[i]);
    include_path[i] = null;

    var errormsg = [null];
    var ret = cpp(String(code), errormsg, include_path);
    if(errormsg[0] != null) {
      if(ret) free(ret);
      var error = errormsg[0].string();
      free(errormsg[0]);
      throw error;
    }
    ret2 = ret.string();
    free(ret);
    return ret2;
  }
})()

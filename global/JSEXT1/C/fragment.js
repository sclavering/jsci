/*
obj = fragment(filename)

Examines a .h file and returns an object containing all the symbols declared there that are exported in .so files it refers to via the appopriate #pragma.

### Arguments ###

* code_str: A string containing C source code, from a .h file
* default_dl: A Dl object where pointers can be resolved

### Return value ###

An object containing strings, which is JavaScript source code
representing each symbol exported by the C program. Symbols
are not exported if they generate code and can not be resolved
in _default\_dl_ or any library mentioned in #pragma directives.
*/
function(filename) {
  var parsed = parse(new XML(jsxcore.cToXML(runcpp(filename))));
  var src = {};

  for(var i in parsed.structs_and_unions)
    if(parsed.exported_symbols[i])
      src[i] = parsed.structs_and_unions[i];

  for(var i in parsed.sym) {
    if(parsed.exported_symbols[i]) {
      if(src[i]) {
        src[i] = "this['" + i + "']=" + src[i] + ";" + parsed.sym[i];
      } else {
        src[i] = parsed.sym[i];
      }
    }
  }

  return src;
}

/*

      obj = fragment (code, default_dl)

  Examines a C program using [[$curdir.parse]] and returns an
  object containing all the symbols exported by the program.

  ### Arguments ###

  * _code_: An XML object as returned from [[$curdir.ctoxml]]
  * _default\_dl_: A Dl object where pointers can be resolved

  ### Return value ###

  An object containing strings, which is JavaScript source code
  representing each symbol exported by the C program. Symbols
  are not exported if they generate code and can not be resolved
  in _default\_dl_ or any library mentioned in #pragma directives.

 */

  function (code, default_dl) {
    var parsed=parse(code, default_dl);
    
    var src={};
    
    for (var i in parsed.su)
      if (parsed.expsym[i])
	src[i]=parsed.su[i];
    
    for (var i in parsed.sym) {
      if (parsed.expsym[i]) {
	if (src[i]) {
	  src[i] = "(this['" + i + "']=" + src[i] + "," + parsed.sym[i] + ")";
	} else {
	  src[i] = parsed.sym[i];
	}
      }
    }
    
    return src;
  }

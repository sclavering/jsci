/*
      source = text_code (code, default_dl)

  Examines a C program using [[$curdir.parse]] and returns
  JavaScript source code which, when evaluated,
  returns an object representing all the exported symbols
  in the C program. Symbols
  are not exported if they generate code and can not be resolved
  in _default\_dl_ or any library mentioned in #pragma directives.

  ### Arguments ###

  * _code_: An XML object as returned from [[$curdir.ctoxml]]
  * _default\_dl_: A Dl object where pointers can be resolved

  ### Return value ###

  A string.
 */

function(code, default_dl) {
  var parsed=parse(code, default_dl);

  var src=[];

  for (var i in parsed.su)
    if (parsed.expsym[i])
      src.push("this['"+i+"']="+parsed.su[i]+";\n");

  for (var j=0; j<parsed.symOrder.length; j++) {
    i=parsed.symOrder[j];
    if (parsed.expsym[i])
      src.push("this['"+i+"']="+parsed.sym[i]+";\n");
  }

  return "(function() {\n"+src.join('')+"return this;\n}).call({});";
}

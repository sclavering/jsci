/*
     names = callify (statements)

 Changes a parsed js script so that if the last statement is a
 function, it is called with the arguments given in $JSEXT_args. The result is
 stored in a variable named $JSEXT_tmp.

 ### Arguments ###

 _statements_ is an array returned from [[$curdir.parse]]
 and which can be passed to [[$curdir.unparse]].

 _statements_ is modified.

 ### Return value ###

 Returns an array containing the argument names of the function,
 if a function definition was indeed the last statement in _statements_.

 */


function(statements) {
  var last=statements[statements.length-1];
  if (last.id!=="function" || last.func['(name)']!==undefined)
    return; // Can't help you if last statement ain't function

  var params=last.func['(params)'];
  if (params!==undefined)
    var names=params.replace(/^ */,"").replace(/ *$/,"").split(/ *, */);
  else
    var names=[];

  statements[statements.length-1]={
    left: {
      type: '(identifier)',
      value: '$JSEXT_tmp'
    },
    id: '=',
    right: {
      left: {
	id: '(',
	expr: last
      },
      id: '.',
      right: "apply(this, $JSEXT_args)"
    }
  }

  return names;
}

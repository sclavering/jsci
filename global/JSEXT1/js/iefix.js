/*

     iefix (statements)

 Fixes a parsed js script so that the last statement is
 treated as an expression also when it is an anonymous function
 and the script is evaluated by ie's eval() function.

 _statements_ is an array returned from [[$curdir.parse]]
 and which can be passed to [[$curdir.unparse]].

 _statements_ is modified.

*/

function(statements) {
  var last=statements[statements.length-1];
  if (last.id=="function") {
    statements[statements.length-1]={
      id: '||',
      left: {
	id: '(',
	expr: last
      },
      right: {
	id: 'undefined'
      }
    }
  }
}

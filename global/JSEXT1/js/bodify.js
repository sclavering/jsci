/*
     bodify (statements)
  
 Changes a parsed js script into a function body, in that a "return"
 keyword is inserted before the last statement.

 _statements_ is an array returned from [[$curdir.parse]]
 and which can be passed to [[$curdir.unparse]].

 _statements_ is modified.

 */

function(statements) {
  var last=statements[statements.length-1];
  statements[statements.length-1]={
      id: 'return',
      expr: last
  }
}

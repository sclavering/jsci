/*
        statements = withify(statements2, inwith);

    Encloses the body of all functions in a parsed js script in
    a _with_ statement.

    ### Arguments ###

    * _statements2_: A parse tree, as returned from [[$curdir.parse]].
    * _inwith_: A string to put inside the _with (.)_ parens.

    ### Return value ###

    A parse tree which can be passed to [[$curdir.unparse]].
    Alters _statements2_.
*/


function(statements, inwith) {
  for (var i=0; i<statements.length; i++) {
    var stmt=statements[i];
    if (stmt.id=="function") {
      stmt.func['(body)']=[{
      id: 'with',
      expr: {
	type: '(identifier)',
	value: inwith
      },
      block: arguments.callee(stmt.func['(body)'], inwith)
      }];
    }
  }
  return statements;
  /*
[{
      id: 'with',
      expr: {
	type: '(identifier)',
	value: inwith
      },
      block: statements
      }
    ];
  */

}

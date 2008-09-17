/*
    db.prepare(qry, [param, param, ...])

Performs an unbuffered query, i.e. one where the
whole result is not stored in memory at once.
Return a new [[$parent.Stmt]] object.
Parameter substitution works the same
as with [[$curdir.query]].

    var res = db.prepare(qry);
    var row;
    while (row = res.row()) {
      ...
    }
    res.free();

Parameters may also be bound after calling _prepare_, but before
calling _row_, using the statement's [[$parent.Stmt.prototype.exec]]
or [[$parent.Stmt.prototype.bind]] methods.

See also [[$parent.Stmt.prototype.free]] and
[[$parent.Stmt.prototype.row]].
 */

function(qry) {
  if (!this.db)
    throw new Error("Sqlite: not connected");
  var utf=$parent.$parent.encodeUTF8(qry);
  var ptr=[null];
  if ($parent.lib.sqlite3_prepare_v2(this.db,
				     utf,
				     utf.length+1,
				     ptr,
				     null))
    this.throwError();

  var ret=new $parent.Stmt(this, ptr[0]);
  for (var i=1; i<arguments.length; i++)
    ret.bind(i, arguments[i]);
      
  return ret;
}

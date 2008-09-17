/*
    db.prepare(qry, [param, param, ...])

Performs an unbuffered query, i.e. one where the
whole result is not stored in memory at once.
Return a [[$parent.Connection]] object, creating a new
one if necessary. Parameter substitution works the same
as with [[$curdir.query]].

    var res = db.prepare(qry);
    var row;
    while (row = res.row()) {
      ...
    }
    res.free();

See also [[$parent.Connection.prototype.free]] and
[[$parent.Connection.prototype.row]].

*/

function() {
  var conn=this.getConnection();

  conn.exec.apply(conn, arguments);

  conn.result=lib.mysql_use_result(conn.mysql);
  if (!conn.result) {
    conn.free();
    conn.throwError();
  }
  
  conn.rowNumber=0;

  conn.result.finalize=lib.mysql_free_result;
  conn.getFields();
  return conn;
}

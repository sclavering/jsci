/*
      db.exec(qry, [param1, [param2]])

  Executes a query which does not return data. Use this with
  UPDATE, INSERT etc. Parameter substitution works the same
  as with [[$curdir.query]].
 */


function(qry) {
  var conn=this.getConnection();
  conn.exec.apply(conn, arguments);
  conn.free();
}

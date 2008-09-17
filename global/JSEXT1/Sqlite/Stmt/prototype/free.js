/*
      stmt.free()

  Frees the resources associated with a Sqlite statement. Use with
  the [[$parent.$parent.prototype.prepare]] function:

      var res = db.prepare(qry);
      var row;
      while (row = res.row()) {
        ...
      }
      res.free();

 */

function() {
  if (this.stmt) {
    $parent.$parent.lib.sqlite3_finalize(this.stmt);
    this.stmt.finalize=null;
    delete this.stmt;
  }
}

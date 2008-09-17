/*
      conn.free()

  Frees the resources associated with a MySQL result set. Use with
  the [[$parent.$parent.prototype.prepare]] function:

      var res = db.prepare(qry);
      var row;
      while (row = res.row()) {
        ...
      }
      res.free();

 */

function() {
  if (this.result) {
    lib.mysql_free_result(this.result);
    this.result.finalize=null;
    delete this.result;
  }
  this.parent.freeConnections.push(this);
}

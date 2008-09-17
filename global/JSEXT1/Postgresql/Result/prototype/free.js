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
  if (!this.closed) {
    this.closed=true;
    lib.PQclear(this.res);
    this.res.finalize=null;
  }
}

function() {
  if ($parent.$parent.lib.sqlite3_reset(this.stmt))
    this.conn.throwError();
}

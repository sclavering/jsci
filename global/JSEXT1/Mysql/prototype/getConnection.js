function() {
  var conn=this.freeConnections.pop();
  if (!conn) {
    conn = new $parent.Connection(this, this.params);
    this.connections.push(conn);
  }
  if (conn.result)
    conn.free();
  return conn;
}

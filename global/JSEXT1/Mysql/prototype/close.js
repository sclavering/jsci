/*
    db.close()

Close all connections from this object to the database server.
The object can still be used, in which case new connections will be
made.

*/

function() {
  var conn;
  
  while ((conn=this.connections.pop()))
    conn.close();

  this.freeConnections = [];
}

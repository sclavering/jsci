/*
      str = sess.newId()

  Creates a random id string which is not currently in use. Stores the string
  in the database so it will not be reused by other sessions until this session
  expires.
 */


function() {
  for (var tries=0; tries<3; tries++) {
    var id=Math.floor(Math.random()*1e42).toString(36);
    try {
      var now=new Date().valueOf()/1000;
      this.db.query("insert into "+this.table+" (timestamp, id, value) values (?,?,'{}')", now, id);
      break;
    } catch(x) {
      // because id exists
    }
  }
  return id;
}

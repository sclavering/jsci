/*
      sess.save(idStr, valueObj)

  JSON-encodes and saves an object with session variables to the database.
 */

function(id, value) {
  var now=new Date().valueOf()/1000;
  this.db.exec("update "+this.table+" set timestamp=?, value=? where id=?", now, $parent.$parent.$parent.encodeJSON(value), id);
}

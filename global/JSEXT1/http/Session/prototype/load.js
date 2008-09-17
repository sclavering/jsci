/*
      obj = sess.load( idStr )

  Retrieves and decodes a JSON-encoded object with session variables from the
  database. New sessions will return empty objects. Non-existing ids will
  return _undefined_.

  Before loading the object, this function deletes all other
  sessions which have expired from the database.
 */

function(id) {
  var now=new Date().valueOf()/1000;
  this.db.exec("delete from "+this.table+" where timestamp < ?",now-this.linger);
  var val=this.db.query("select value from "+this.table+" where id=?",id)[0];
  if (!val)
    return;

  return $parent.$parent.$parent.decodeJSON(val.value);
}

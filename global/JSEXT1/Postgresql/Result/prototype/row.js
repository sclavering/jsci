/*
  Returns one row from the query: An object
  whose keys are field names and values are field values.
  
  If there are no more fields, the return value is _undefined_.

  Use with
  the [[$parent.$parent.prototype.prepare]] function:

      var res = db.prepare(qry);
      var row;
      while (row = res.row()) {
        ...
      }
      res.free();

*/

function() {
  if (this.pos>=this.length)
    return;
  var ret={};
  for (var i=0; i<this.names.length; i++) {
    var cp=lib.PQgetvalue(this.res, this.pos, i);
    cp=cp.string(lib.PQgetlength(this.res, this.pos, i));
    if (cp.length==0 && lib.PQgetisnull(this.res, this.pos, i))
      cp=null;
    else if (this.types[i])
      cp=this.types[i](cp);
    ret[this.names[i]]=cp;
  }
  this.pos++;
  return ret;
}

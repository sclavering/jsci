/*
InsertionValues

A helper type to allow using substitution in an INSERT.

Doing _db.query("INSERT table_foo (col_1, col_2) VALUES ?", x)_ fails if _x_ is
an array, because it is substituted with an excess pair of parentheses.  Using
_new InsertionValues(x)_ instead solves this.
*/
(function() {

function InsertionValues(array) {
  if(!(this instanceof InsertionValues)) return new InsertionValues(array);
  this.array = array;
}

InsertionValues.prototype = {
  toMysqlString: function(db) {
    return db.quote_array(this.array, true);
  },
}

return InsertionValues;

})()

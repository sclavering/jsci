/*

      stmt.exec([param1, [param2, ...]])

  Binds parameters and returns all rows. Useful when inserting
  several rows with the same query, replacing only the parameters.
  Returns the rows resulting from the query in the same
  fashion as [[$parent.$parent.prototype.query]].

*/

function() {
  this.reset();

  for (var i=0; i<arguments.length; i++)
    this.bind(i+1, arguments[i]);

  var ret=[];
  for (;;) {
    var row=this.row();
    if (row===undefined) break;
    ret.push(row);
  }

  return ret;
}

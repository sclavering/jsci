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
  function row() {
    var lib=$parent.$parent.lib;
    
    if (!this.columnTypes) {
      var i=0;
      this.columnTypes=[];
      this.columnNames=[];
      var nfields = lib.sqlite3_column_count(this.stmt);
      for (i=0; i<nfields; i++) {
	this.columnTypes[i]=lib.sqlite3_column_type(this.stmt, i);
	this.columnNames[i]=lib.sqlite3_column_name16(this.stmt, i).UCstring();
      }
    }

    var nfields=this.columnTypes.length;
    if (nfields===0)
      return;
    
    var ret={};
    var col;
    var ptr;
    for (i=0; i<nfields; i++) {
      switch(this.columnTypes[i]) {
      case lib.SQLITE_INTEGER:
	col=lib.sqlite3_column_int(this.stmt, i);
	break;
      case lib.SQLITE_FLOAT:
	col=lib.sqlite3_column_double(this.stmt, i);
	break;
      case lib.SQLITE_TEXT:
	ptr=lib.sqlite3_column_text16(this.stmt, i);
	if (ptr==null)
	  col=null;
	else
	  col=ptr.UCstring(lib.sqlite3_column_bytes16(this.stmt, i)/2);
	break;
      case lib.SQLITE_BLOB:
	ptr=lib.sqlite3_column_blob(this.stmt, i);
	col=new $parent.$parent.$parent.StringFile(ptr.string(lib.sqlite3_column_bytes(this.stmt, i)));
	break;
      case lib.SQLITE_NULL:
	col=null;
	break;
      }
      
      ret[this.columnNames[i]]=col;
    }
    
    return ret;
  }

  var lib=$parent.$parent.lib;
  var r=lib.sqlite3_step(this.stmt);
  switch (r) {
  case lib.SQLITE_BUSY:
    throw "busy";
  case lib.SQLITE_ROW:
    return row.call(this);
    break;
  case lib.SQLITE_DONE:
    return;
    break;
  case lib.SQLITE_ERROR:
    this.conn.throwError();
  case lib.SQLITE_MISUSE:
    throw new Error("Sqlite: misuse");
  }

}

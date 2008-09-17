/*
      stmt.bind(n, value)

  Binds a value to a parameter. Parameters are
  question marks in the query string.

  File-like values (objects having _read_ and _close_ properties)
  are bound as blobs (8-bit binary objects).
  They are read and closed.

  ### Arguments ###

  * _n_: The index of the argument to bind. 1 is the first question
    mark in the query string.
  * _value_: A string, date, number, File-like object or null value.

  ### Version differences ###

  Version 1.0 had an optional third argument:

  * _blob_: Optional. If true, and _value_ is a string, the string
    characters are treated as if 8 bits wide.

  This argument is still available in version >= 1.1, but it is now
  deprecated. Use file-like objects to indicate binary data instead.

 */

function(n, value, blob) {
  switch(typeof value) {
  case 'number':
    $parent.$parent.lib.sqlite3_bind_double(this.stmt, n, value);
    break;
  case 'object':
    if (value===null) {
	$parent.$parent.lib.sqlite3_bind_null(this.stmt, n);
      break;
    } else if (typeof value.read == "function") {
      var tmp=value;
      value = tmp.read();
      tmp.close();
      blob=true;
    } else if (value instanceof Date) {
      value = String(value);
    }

    // fall through

  case 'string':
    if (blob) {
      $parent.$parent.lib.sqlite3_bind_blob(this.stmt,
					    n,
					    Type.pointer(Type.char),
					    value,
					    value.length,
					    Type.int,
					    $parent.$parent.lib.SQLITE_TRANSIENT);
    } else {
      $parent.$parent.lib.sqlite3_bind_text16(this.stmt,
					      n,
					      Type.pointer(Type.short),
					      value,
					      value.length*2,
					      Type.int,
					      $parent.$parent.lib.SQLITE_TRANSIENT);
    }
    break;
  }
}


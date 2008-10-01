/*
    array = db.query(qry, [param, param, ...])

Return an array of rows if the query is of a type that can return rows.
Otherwise, returns _undefined_. I.e. an query which can return rows
(like a SELECT statement),
but which returns zero rows will return a zero-length array. A query
which can not return rows (like an UPDATE statement) will return
_undefined_.

### Parameter substitution ###

Each question mark in the query string (which is not inside quotes,
version >= 1.1)
is replaced with the corresponding
parameter in the argument list, following the query string. Parameters
are properly escaped during substitution.

Question marks immediately followed by a number (?1 ?2 ?3) are replaced
by the corresponding parameter number, starting at 1. Subsequent question
marks without numbers are replaced by the parameter following the
highest-numbered parameter used so far. (version >= 1.1)

---

**Example**

---

    var ids=db.query("SELECT id FROM person WHERE name=? AND age=?",
                    "o'hara", 42);

---

### Return value ###

If the query _can_ return rows, the return value is an array, where
each element represents a row.

Each row is an object
with keys equal to the column names.

### Blobs ###

Blobs (8 bit wide binary objects) should be passed as file-like objects
(having a _read_ and _close_ property) in the parameter list. These
objects are read and closed.

Likewise, blobs in result sets are returned as file-like objects
([[JSEXT1.StringFile]]).

---

**Example**

---

    var res=db.query("SELECT id, age FROM person WHERE name=?",
                    "o'hara");

    print(res[0].id); // Prints a number or throws an exception

---


### Version differences ###

* Version 1.0 also replaced question marks inside strings
* Support for 16-bit characters was introduced in version 1.1.
* Support for file-like blobs was introduced in version 1.1.

*/

function(qry) {
  var conn=this.getConnection();
  
  conn.exec.apply(conn, arguments);
  
  if (lib.mysql_field_count(conn.mysql)==0)
    return lib.mysql_insert_id(conn.mysql) || null;
  
  conn.result=lib.mysql_store_result(conn.mysql);
  if (!conn.result) {
    conn.release();
    conn.throwError();
  }
  conn.result.finalize=lib.mysql_free_result;
  
  conn.rowNumber=0;
  conn.getFields();
  
  var ret=[];
  var row;
  while ((row=conn.row())!==undefined) {
    ret.push(row);
  }
  
  conn.free();
  
  return ret;
}

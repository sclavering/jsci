/*
    array = db.query(qry, [param, param, ...])

Return an array of rows.

### Parameter substitution ###

Each question mark in the query string is replaced with the corresponding
parameter in the argument list, following the query string. Parameters
are properly escaped during substitution.

---

**Example**

---

    var ids=db.query("SELECT id FROM person WHERE name=? AND age=?",
                    "o'hara", 42);

---

### Return value ###

The return value is an array, where
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
*/

function(qry) {
  var stmt=this.prepare(qry);

  for (var i=1; i<arguments.length; i++)
    stmt.bind(i, arguments[i]);

  var ret=[];

  for (;;) {
    var row=stmt.row();
    if (row===undefined) break;
    ret.push(row);
  }

  stmt.free();

  return ret;
}

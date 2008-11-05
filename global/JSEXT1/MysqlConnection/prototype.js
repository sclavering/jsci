({
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
  query: function(qry) {
    const lib = libmysql, conn = this;
    this._exec(arguments);

    if(lib.mysql_field_count(conn.mysql) == 0) {
      conn.free();
      return lib.mysql_insert_id(conn.mysql) || null;
    }

    conn.result = lib.mysql_store_result(conn.mysql);
    if(!conn.result) {
      conn.release();
      conn.throwError();
    }
    conn.result.finalize = lib.mysql_free_result;

    conn.rowNumber = 0;
    conn.getFields();

    const ret = [];
    var row;
    while((row = conn.row()) !== undefined) ret.push(row);
    conn.free();
    return ret;
  },


  /*
  db.exec(qry, [param1, [param2]])

  Executes a query which does not return data. Use this with
  UPDATE, INSERT etc. Parameter substitution works the same
  as with [[$curdir.query]].
  */
  exec: function(qry) {
    this._exec(arguments);
    this.free();
  },


  close: function() {
    const lib = libmysql;
    if(this.result) this.free();
    lib.mysql_close(this.mysql);
    this.mysql.finalize = null;
    delete this.mysql;
  },


  // internal!
  _exec: function(args) {
    const lib = libmysql;
    var qry = args[0];
    var maxarg = 0;
    var inquote = false;
    var self = this;

    if(!this.mysql) throw new Error("Not connected");

    qry = $parent.encodeUTF8(qry).replace(/(\\?[\"\'])|(\?([0-9]*))/g, replaceFunc);

    /*
    stderr.write('Query log: ');
    stderr.write(qry);
    stderr.write("\n");
    */

    var res = lib.mysql_real_query(this.mysql, qry, qry.length);
    if(res) this.throwError();

    function replaceFunc(q, a, b, c) {
      if(a) {
        if(a[0] != '\\') {
          if(inquote) {
            if(inquote == a)
              inquote = false;
          } else {
            inquote = a;
          }
        }
        return q;
      }

      if(inquote) return q;

      if(c) {
        c = Number(c);
        var val = args[c];
        if(c > maxarg) maxarg = c;
      } else {
        var val = args[++maxarg];
      }
      return self.quote_value(val);
    }
  },


  /*
  */
  quote_value: function(val) {
    if(val === null || val === undefined) return 'null';
    const type = typeof(val);
    if(type == "number" || type == "boolean") return String(val);
    // String would match the array case below
    if(type == "object" && !(val instanceof String)) {
      if(val instanceof Date) return "'" + val.toLocaleFormat('%Y-%m-%d %H:%M:%S') + "'";

      if(typeof val.read == "function") {
        var str = val.read();
        val.close();
        var to = Pointer.malloc(str.length * 2 + 1);
        var len = lib.mysql_real_escape_string(this.mysql, to, str, str.length);
        return "'" + to.string(len) + "'";
      }

      if(typeof val.length == "number") return this.quote_array(val);
    }

    // Use a safe default of stringifying then escaping then quoting
    val = String(val);
    val = $parent.encodeUTF8(val);
    var to = Pointer.malloc(val.length * 2 + 1);
    var len = libmysql.mysql_real_escape_string(this.mysql, to, val, val.length);
    return "'" + to.string(len) + "'";
  },

  quote_array: function(values) {
    const quoted = [];
    for(var i = 0; i != values.length; ++i) quoted[i] = this.quote_value(values[i]);
    return '(' + quoted.join(', ') + ')';
  },


  /*
  conn.free()

  Frees the resources associated with a MySQL result set. Use with
  the [[$parent.prototype.prepare]] function:

      var res = db.prepare(qry);
      var row;
      while (row = res.row()) {
        ...
      }
      res.free();
  */
  free: function() {
    const lib = libmysql;
    if(this.result) {
      lib.mysql_free_result(this.result);
      this.result.finalize = null;
      delete this.result;
    }
  },


  getFields: function() {
    const lib = libmysql;
    this.fieldNames = [];
    this.fieldTypes = [];
    this.fieldCharSet = [];

    var field;
    while((field = lib.mysql_fetch_field(this.result)) != null) {
      this.fieldTypes.push(field.member(0, 'type').$);
      this.fieldCharSet.push(field.member(0, 'charsetnr').$);
      this.fieldNames.push(field.member(0, 'name').$.string(field.member(0, 'name_length').$));
    }
  },


  /*
  Returns one row from the query: An object
  whose keys are field names and values are field values.

  If there are no more fields, the return value is _undefined_.

  Use with
  the [[$parent.prototype.prepare]] function:

      var res = db.prepare(qry);
      var row;
      while (row = res.row()) {
        ...
      }
      res.free();
  */
  row: function() {
    const lib = libmysql;
    var nfields = this.fieldNames.length;

    var row = lib.mysql_fetch_row(this.result);
    if(!row) return;

    var outval;

    var lengths = lib.mysql_fetch_lengths(this.result);

    var outrow = {};
    for(var j = 0; j < nfields; j++) {
      val = row.member(j).$;
      if(val) {
        val = val.string(lengths.member(j).$);

        switch(this.fieldTypes[j]) {
          case lib.MYSQL_TYPE_DATE:
            var date = val.match(/([0-9]*)-([0-9]*)-([0-9]*)/);
            outval = new Date(date[1], date[2], date[3]);
            break;
          // case lib.MYSQL_TYPE_TIME:
          //   var date = val.match(/([0-9]*):([0-9]*):([0-9]*)/);
          //   outval = new Date(new Date(1970, 0, 1, date[1], date[2], date[3]).valueOf() % (24 * 3600000));
          //   break;
          case lib.MYSQL_TYPE_DATETIME:
            var date = val.match(/([0-9]*)-([0-9]*)-([0-9]*) ([0-9]*):([0-9]*):([0-9]*)/);
            outval = new Date(date[1], date[2] - 1, date[3], date[4], date[5], date[6]);
            break;
          case lib.MYSQL_TYPE_DECIMAL:
          case lib.MYSQL_TYPE_TINY:
          case lib.MYSQL_TYPE_SHORT:
          case lib.MYSQL_TYPE_LONG:
          case lib.MYSQL_TYPE_FLOAT:
          case lib.MYSQL_TYPE_DOUBLE:
          case lib.MYSQL_TYPE_LONGLONG:
          case lib.MYSQL_TYPE_INT24:
            outval = Number(val);
            break;
          case lib.MYSQL_TYPE_TINY_BLOB:
          case lib.MYSQL_TYPE_MEDIUM_BLOB:
          case lib.MYSQL_TYPE_LONG_BLOB:
          case lib.MYSQL_TYPE_BLOB:
            if(this.fieldCharSet[j] == 63) {
              outval = new $parent.StringFile(val);
              break;
            } 
            // is a text field, fall through
          case lib.MYSQL_VAR_STRING:
          case lib.MYSQL_STRING:
          case lib.MYSQL_VARCHAR:
            outval = $parent.decodeUTF8(val);
            break;
          default:
            outval = val;
        }
      } else {
        outval = null;
      }

      outrow[this.fieldNames[j]] = outval;
    }

    this.rowNumber++;
    return outrow;
  },


  /*
  num = conn.rowCount()

  Returns the number of rows in the result set.
  */
  rowCount: function() {
    const lib = libmysql;
    if(this.result) return lib.mysql_num_rows(this.result);
  },


  /*
  conn.seek(num)

  Moves the cursor backwards or forwards in the result set.
  */
  seek: function(number) {
    const lib = libmysql;
    lib.mysql_data_seek(this.result, number);
    this.rowNumber = number;
  },


  // Used for reporting errors from the mysql api (private)
  throwError: function() {
    const lib = libmysql;
    throw new Error("mysql: " + $parent.decodeUTF8(lib.mysql_error(this.mysql).string()));
  },
})

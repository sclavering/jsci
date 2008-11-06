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

  Each question mark in the query string is replaced with the corresponding
  parameter in the argument list, following the query string.  Parameters
  are properly escaped during substitution.

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
    this._exec(arguments);

    if(libmysql.mysql_field_count(this._mysql) == 0) {
      this.free();
      return libmysql.mysql_insert_id(this._mysql) || null;
    }

    this.result = libmysql.mysql_store_result(this._mysql);
    if(!this.result) {
      this.throwError();
    }
    this.result.finalize = libmysql.mysql_free_result;

    this.rowNumber = 0;
    this.getFields();

    const ret = [];
    var row;
    while((row = this.row()) !== undefined) ret.push(row);
    this.free();
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


  insert_object: function(table, object) {
    return this.insert_multiple_objects(table, [object]);
  },


  insert_multiple_objects: function(table, object_list) {
    if(!object_list || !object_list.length) return null;
    const keys = object.keys(object_list[0]);
    const row_len = keys.length;
    const data = new Array();
    for(var i = 0; i != object_list.length; ++i) {
      var obj = object_list[i];
      var row = data[i] = new Array(row_len);
      for(var k = 0; k != row_len; ++k) row[k] = obj[keys[k]] || '';
    }
    return this.insert_multiple_rows(table, keys, data);
  },


  insert_multiple_rows: function(table, field_names, data) {
    const q = 'INSERT INTO ' + table + ' (' + field_names.join(', ') + ') VALUES ';
    const escaped = new Array(data.length);
    for(var i = 0; i != data.length; ++i) escaped[i] = this.quote_array(data[i]);
    return this.query(q + escaped.join(', '));
  },


  close: function() {
    if(this.result) this.free();
    libmysql.mysql_close(this._mysql);
    this._mysql.finalize = null;
    delete this._mysql;
  },


  // internal!
  _exec: function(args) {
    var qry = args[0];
    var maxarg = 0;
    var self = this;

    if(!this._mysql) throw new Error("Not connected");

    qry = $parent.encodeUTF8(qry).replace(/\?/g, replaceFunc);

    /*
    stderr.write('Query log: ');
    stderr.write(qry);
    stderr.write("\n");
    */

    var res = libmysql.mysql_real_query(this._mysql, qry, qry.length);
    if(res) this.throwError();

    function replaceFunc(q, a, b, c) {
      var val = args[++maxarg];
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
        var len = libmysql.mysql_real_escape_string(this._mysql, to, str, str.length);
        return "'" + to.string(len) + "'";
      }

      if(typeof val.length == "number") return this.quote_array(val);
    }

    // Use a safe default of stringifying then escaping then quoting
    val = String(val);
    val = $parent.encodeUTF8(val);
    var to = Pointer.malloc(val.length * 2 + 1);
    var len = libmysql.mysql_real_escape_string(this._mysql, to, val, val.length);
    return "'" + to.string(len) + "'";
  },

  quote_array: function(values) {
    const quoted = [];
    for(var i = 0; i != values.length; ++i) quoted[i] = this.quote_value(values[i]);
    return '(' + quoted.join(', ') + ')';
  },


  /*
  mysql.free()

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
    if(this.result) {
      libmysql.mysql_free_result(this.result);
      this.result.finalize = null;
      delete this.result;
    }
  },


  getFields: function() {
    this.fieldNames = [];
    this.fieldTypes = [];
    this.fieldCharSet = [];

    var field;
    while((field = libmysql.mysql_fetch_field(this.result)) != null) {
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
    var nfields = this.fieldNames.length;

    var row = libmysql.mysql_fetch_row(this.result);
    if(!row) return;

    var outval;

    var lengths = libmysql.mysql_fetch_lengths(this.result);

    var outrow = {};
    for(var j = 0; j < nfields; j++) {
      val = row.member(j).$;
      if(val) {
        val = val.string(lengths.member(j).$);
        outval = this._decode_result_value(val, this.fieldTypes[j], this.fieldCharSet[j]);
      } else {
        outval = null;
      }

      outrow[this.fieldNames[j]] = outval;
    }

    this.rowNumber++;
    return outrow;
  },

  _decode_result_value: function(val, field_type, field_charset) {
    switch(field_type) {
      case libmysql.MYSQL_TYPE_DATE:
        var date = val.match(/([0-9]*)-([0-9]*)-([0-9]*)/);
        return new Date(date[1], date[2], date[3]);
      // case libmysql.MYSQL_TYPE_TIME:
      //   var date = val.match(/([0-9]*):([0-9]*):([0-9]*)/);
      //   return new Date(new Date(1970, 0, 1, date[1], date[2], date[3]).valueOf() % (24 * 3600000));
      case libmysql.MYSQL_TYPE_DATETIME:
        var date = val.match(/([0-9]*)-([0-9]*)-([0-9]*) ([0-9]*):([0-9]*):([0-9]*)/);
        return new Date(date[1], date[2] - 1, date[3], date[4], date[5], date[6]);
      case libmysql.MYSQL_TYPE_DECIMAL:
      case libmysql.MYSQL_TYPE_TINY:
      case libmysql.MYSQL_TYPE_SHORT:
      case libmysql.MYSQL_TYPE_LONG:
      case libmysql.MYSQL_TYPE_FLOAT:
      case libmysql.MYSQL_TYPE_DOUBLE:
      case libmysql.MYSQL_TYPE_LONGLONG:
      case libmysql.MYSQL_TYPE_INT24:
        return Number(val);
      case libmysql.MYSQL_TYPE_TINY_BLOB:
      case libmysql.MYSQL_TYPE_MEDIUM_BLOB:
      case libmysql.MYSQL_TYPE_LONG_BLOB:
      case libmysql.MYSQL_TYPE_BLOB:
        if(field_charset == 63) return new $parent.StringFile(val);
        // it's a text field, fall through
      case libmysql.MYSQL_VAR_STRING:
      case libmysql.MYSQL_STRING:
      case libmysql.MYSQL_VARCHAR:
        return $parent.decodeUTF8(val);
    }
    return val;
  },


  /*
  num = mysql.rowCount()

  Returns the number of rows in the result set.
  */
  rowCount: function() {
    if(this.result) return libmysql.mysql_num_rows(this.result);
  },


  /*
  mysql.seek(num)

  Moves the cursor backwards or forwards in the result set.
  */
  seek: function(number) {
    libmysql.mysql_data_seek(this.result, number);
    this.rowNumber = number;
  },


  // Used for reporting errors from the mysql api (private)
  throwError: function() {
    throw new Error("mysql: " + $parent.decodeUTF8(libmysql.mysql_error(this._mysql).string()));
  },
})

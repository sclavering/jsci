({
  InsertionValues: $curdir.InsertionValues,
  Name: $curdir.Name,

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

  Escaping rules are hard-coded for most built-in types (e.g. arrays, integers
  strings, and Date objects).  Objects can define a _.toMysqlString()_ method
  to control their own escaping during substitution.  If this method exists it
  will be called and the result used unmodified (e.g. no quote marks will be
  added).

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
  */
  query: function(qry) {
    this._exec(arguments);

    if(libmysql.mysql_field_count(this._mysql) == 0) return undefined;

    this.result = libmysql.mysql_store_result(this._mysql);
    if(!this.result) this._throw_error();
    this.result.finalize = libmysql.mysql_free_result;

    const fields = this._get_fields();

    const ret = [];
    while(true) {
      var row = libmysql.mysql_fetch_row(this.result);
      if(!row) break;
      var lengths = libmysql.mysql_fetch_lengths(this.result);
      ret.push(this._decode_result_row(row, lengths, fields));
    }

    this.free();
    return ret;
  },


  /*
  db.insert(qry, [param1, [...]])
  
  Like .exec(), except it returns the result of mysql_insert_id(), avoiding the
  need for a 'SELECT LAST_INSERT_ID()'.
  */
  insert: function(qry) {
    this._exec(arguments);
    this.free();
    return libmysql.mysql_insert_id(this._mysql) || null;
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
    return this.insert('?', { _str: q + escaped.join(', '), toMysqlString: function() { return this._str; } });
  },


  close: function() {
    if(this.result) this.free();
    libmysql.mysql_close(this._mysql);
    this._mysql.finalize = null;
    delete this._mysql;
  },


  _exec: function(args) {
    var qry = args[0];
    var maxarg = 0;
    var self = this;
    const named_args = args.length > 1 ? args[args.length - 1] : {};

    if(!this._mysql) throw new Error("Not connected");

    // We have to escape before doing substitution, because the quoting during
    // substitution uses libmysql functions that expect 8-bit strings.
    qry = $parent.encodeUTF8(qry).replace(/\?|\{([^}]*)\}/g, replace);

    /*
    stderr.write('Query log: ');
    stderr.write(qry);
    stderr.write("\n");
    */

    var res = libmysql.mysql_real_query(this._mysql, qry, qry.length);
    if(res) this._throw_error();

    function replace(full_match, name_match) {
      return full_match === '?' ? replace_by_position() : replace_by_name('', name_match);
    }

    function replace_by_position(q, a, b, c) {
      var val = args[++maxarg];
      return self.quote_value(val);
    }

    function replace_by_name(full_match, name_match) {
      if(!(name_match in named_args)) throw new Error("MysqlConnection: missing named query arg: " + name_match);
      return self.quote_value(named_args[name_match]);
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
      if("toMysqlString" in val) return val.toMysqlString(this);

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


  quote_array: function(values, without_parentheses) {
    const quoted = [];
    for(var i = 0; i != values.length; ++i) quoted[i] = this.quote_value(values[i]);
    if(without_parentheses) return quoted.join(', ');
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


  _get_fields: function() {
    const fm = [];
    while(true) {
      var field = libmysql.mysql_fetch_field(this.result);
      if(!field) break;
      fm.push({
        mysql_type: field.member(0, 'type').$,
        charset: field.member(0, 'charsetnr').$,
        name: field.member(0, 'name').$.string(field.member(0, 'name_length').$),
      });
    }
    return fm;
  },


  _decode_result_row: function(row, lengths, fields) {
    var outrow = {};
    for(var j = 0; j != fields.length; ++j) {
      var outval = null;
      var field = fields[j];
      val = row.member(j).$;
      if(val) {
        val = val.string(lengths.member(j).$);
        outval = this._decode_result_value(val, field);
      }
      outrow[field.name] = outval;
    }
    return outrow;
  },


  _decode_result_value: function(val, field) {
    switch(field.mysql_type) {
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
      case libmysql.MYSQL_TYPE_NEWDECIMAL: // Precision math DECIMAL or NUMERIC field (MySQL 5.0.3 and up)
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
        if(field.charset == 63) return new $parent.StringFile(val);
        // it's a text field, fall through
      case libmysql.MYSQL_TYPE_VAR_STRING: // VARCHAR or VARBINARY field
      case libmysql.MYSQL_TYPE_STRING: // CHAR or BINARY per docs, but also TEXT
        return val;
    }
    return val;
  },

/*
MYSQL_TYPE_BIT: // BIT field (MySQL 5.0.3 and up)
MYSQL_TYPE_TIMESTAMP: // TIMESTAMP field
MYSQL_TYPE_YEAR: // YEAR field
MYSQL_TYPE_SET: // SET field
MYSQL_TYPE_ENUM: // ENUM field
MYSQL_TYPE_GEOMETRY: // Spatial field
MYSQL_TYPE_NULL:
*/

  // Used for reporting errors from the mysql api (private)
  _throw_error: function() {
    throw new Error("mysql: " + $parent.decodeUTF8(libmysql.mysql_error(this._mysql).string()));
  },
})

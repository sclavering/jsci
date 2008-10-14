({
  lib: $parent.lib,


  close: function() {
    if(this.result) this.free();
    lib.mysql_close(this.mysql);
    this.mysql.finalize = null;
    delete this.mysql;
  },


  // internal!
  exec: function(qry) {
    var args = arguments;
    var maxarg = 0;
    var inquote = false;
    var self = this;

    qry = $parent.$parent.encodeUTF8(qry).replace(/(\\?[\"\'])|(\?([0-9]*))/g, replaceFunc);

    if(!this.mysql) throw new Error("Not connected");

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
      return escape_val(val);
    }

    function escape_val(val) {
      if(val === null || val === undefined) return 'null';
      switch(typeof(val)) {
        case "number":
        case "boolean":
          return String(val);
        case "string": // Escape and quote string
          val = $parent.$parent.encodeUTF8(val);
          var to = Pointer.malloc(val.length * 2 + 1);
          var len = lib.mysql_real_escape_string(self.mysql, to, val, val.length);
          return "'" + to.string(len) + "'";
        case "object": // Must be date or file
          if(val instanceof Date) {
            return "'" + val.getFullYear() + "-" + val.getMonth() + "-" + val.getDate() + " " + val.getHours() + ":" + val.getMinutes() + ":" + val.getSeconds() + "'";
          }
          if (typeof val.read == "function") {
            var str = val.read();
            val.close();
            var to = Pointer.malloc(str.length * 2 + 1);
            var len = lib.mysql_real_escape_string(self.mysql, to, str, str.length);
            return "'" + to.string(len) + "'";
          }
          if(typeof val.length == "number") {
            return '(' + Array.map(val, escape_val).join(', ') + ')';
          }
          break;
        default:
          return String(val);
      }

      return val;
    }
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
    if(this.result) {
      lib.mysql_free_result(this.result);
      this.result.finalize = null;
      delete this.result;
    }
    this.parent.freeConnections.push(this);
  },


  getFields: function() {
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
              outval = new $parent.$parent.StringFile(val);
              break;
            } 
            // is a text field, fall through
          case lib.MYSQL_VAR_STRING:
          case lib.MYSQL_STRING:
          case lib.MYSQL_VARCHAR:
            outval = $parent.$parent.decodeUTF8(val);
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
    if(this.result) return lib.mysql_num_rows(this.result);
  },


  /*
  conn.seek(num)

  Moves the cursor backwards or forwards in the result set.
  */
  seek: function(number) {
    lib.mysql_data_seek(this.result, number);
    this.rowNumber = number;
  },


  // Used for reporting errors from the mysql api (private)
  throwError: function() {
    throw new Error("mysql: " + $parent.$parent.decodeUTF8(lib.mysql_error(this.mysql).string()));
  },
})

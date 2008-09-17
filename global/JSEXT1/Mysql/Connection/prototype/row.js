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
  var nfields=this.fieldNames.length;
  
  var row=lib.mysql_fetch_row(this.result);
  if (!row)
    return;
  
  var outval;
  
  var lengths=lib.mysql_fetch_lengths(this.result);
  
  var outrow={};
  for (var j=0; j<nfields; j++) {
    val=row.member(j).$;
    if (val) {
      val=val.string(lengths.member(j).$);

      switch(this.fieldTypes[j]) {
      case lib.MYSQL_TYPE_DATE:
	var date=val.match(/([0-9]*)-([0-9]*)-([0-9]*)/);
	outval=new Date(date[1],date[2],date[3]);
	break;
//       case lib.MYSQL_TYPE_TIME:
// 	var date=val.match(/([0-9]*):([0-9]*):([0-9]*)/);
// 	outval=new Date(new Date(1970,0,1,date[1],date[2],date[3]).valueOf()%(24*3600000));
// 	break;
      case lib.MYSQL_TYPE_DATETIME:
	var date=val.match(/([0-9]*)-([0-9]*)-([0-9]*) ([0-9]*):([0-9]*):([0-9]*)/);
	outval=new Date(date[1],date[2]-1,date[3],date[4],date[5],date[6]);
	break;
      case lib.MYSQL_TYPE_DECIMAL:
      case lib.MYSQL_TYPE_TINY:
      case lib.MYSQL_TYPE_SHORT:
      case lib.MYSQL_TYPE_LONG:
      case lib.MYSQL_TYPE_FLOAT:
      case lib.MYSQL_TYPE_DOUBLE:
      case lib.MYSQL_TYPE_LONGLONG:
      case lib.MYSQL_TYPE_INT24:
	outval=Number(val);
	break;
      case lib.MYSQL_TYPE_TINY_BLOB:
      case lib.MYSQL_TYPE_MEDIUM_BLOB:
      case lib.MYSQL_TYPE_LONG_BLOB:
      case lib.MYSQL_TYPE_BLOB:
	if (this.fieldCharSet[j]==63) {
	  outval=new $parent.$parent.$parent.StringFile(val);
	  break;
	} 

	// is a text field, fall through

      case lib.MYSQL_VAR_STRING:
      case lib.MYSQL_STRING:
      case lib.MYSQL_VARCHAR:
	outval=$parent.$parent.$parent.decodeUTF8(val);
	break;
      default:
	outval=val;
      }
    } else
      outval=null;

    outrow[this.fieldNames[j]]=outval;
  }
  
  this.rowNumber++;
  return outrow;
}

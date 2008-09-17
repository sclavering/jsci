//internal!

function(qry) {
  var args=arguments;
  var maxarg=0;
  var inquote=false;
  var self=this;

  qry=$parent.$parent.$parent.encodeUTF8(qry).replace(/(\\?[\"\'])|(\?([0-9]*))/g, replaceFunc);

  if (!this.mysql)
    throw new Error("Not connected");
  
  var res=lib.mysql_real_query(this.mysql, qry, qry.length);
  if (res)
    this.throwError();
  
  function replaceFunc(q,a,b,c) {
    if (a) {
      if (a[0]!='\\') {
	if (inquote) {
	  if (inquote==a)
	    inquote=false;
	} else
	  inquote=a;
      }
      return q;
    }

    if (inquote)
      return q;

    if (c) {
      c=Number(c);
      var val=args[c];
      if (c > maxarg)
	maxarg=c;
    } else {
      var val=args[++maxarg];
    }

    switch(typeof(val)) {
    case "string": // Escape and quote string
      val=$parent.$parent.$parent.encodeUTF8(val);
      var to=Pointer.malloc(val.length*2+1);
      var len=lib.mysql_real_escape_string(self.mysql, to, val, val.length);
      return "'"+to.string(len)+"'";
    case "object": // Must be date or file
      if (val instanceof Date) {
	return "'"+val.getFullYear()+"-"+val.getMonth()+"-"+val.getDate+" "+
	  val.getHours()+":"+val.getMinutes+":"+val.getSeconds+"'";
      } else if (val === null) {
	return "null";
      } else if (typeof val.read == "function") {
	var str=val.read();
	val.close();
	var to=Pointer.malloc(str.length*2+1);
	var len=lib.mysql_real_escape_string(self.mysql, to, str, str.length);
	return "'"+to.string(len)+"'";
      }
      break;
    default:
      return String(val);
    }
    
    return val;
  }
  
}

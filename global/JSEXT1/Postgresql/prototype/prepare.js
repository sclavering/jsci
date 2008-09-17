/*
    db.prepare(qry, [param, param, ...])

Performs a query and returns a
a [[$parent.Result]] object if the query is
of a kind which can return rows (select, show or similar).
Parameter substitution works the same
as with [[$curdir.query]].

    var res = db.prepare(qry);
    var row;
    while (row = res.row()) {
      ...
    }
    res.free();

See also [[$parent.Connection.prototype.free]] and
[[$parent.Connection.prototype.row]].

*/

function(qry) {
  var values=[];
  var lengths=[];
  var types=[];
  var formats=[];
  var maxarg=0;
  var inquote=false;

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
      if (c > maxarg)
	maxarg=c;
      return "$"+c;
    } else {
      return "$"+(++maxarg);
    }
  }

  qry=qry.replace(/(\\?[\"\'])|(\?([0-9]*))/g, replaceFunc);

  for (var i=1; i<arguments.length; i++) {
    switch(typeof arguments[i]) {
    case 'string':
      var val=$parent.$parent.encodeUTF8(arguments[i]);
      types.push(0);
      formats.push(1);
      break;
    case 'object':
      if (val===null) {
	val="null";
      } else if (arguments[i] instanceof Date) {
	var val=sprintf("%04d/%02d/%02d %02d:%02d:%02d%+03d",
			arguments[i].getFullYear(),
			arguments[i].getMonth()+1,
			arguments[i].getDate(),
			arguments[i].getHours(),
			arguments[i].getMinutes(),
			arguments[i].getSeconds(),
			-arguments[i].getTimezoneOffset()/60
			);
	  formats.push(0);
	  types.push(0);
	  break;
      } else if (typeof arguments[i].read == "function") {
	var val=arguments[i].read();//.replace(/\0/g,"\\000");
	arguments[i].close();
	types.push(17); // bytea
	formats.push(1); // binary
      }
      break;
    default:
      var val=String(arguments[i]);
      formats.push(0);
      types.push(0);
    }
    values.push(val);
    lengths.push(val.length);
  }

  var res=lib.PQexecParams(this.conn,
			   qry,
			   arguments.length-1,
			   types,
			   values,
			   lengths,
			   formats,
			   0
			   );

  if (!res)
    throw Error(this.error());

  var status=lib.PQresultStatus(res);

  switch(status) {
  case lib.PGRES_TUPLES_OK:
    return new $parent.Result(res, this);
  case lib.PGRES_COMMAND_OK:
    lib.PQclear(res);
    break;
  default:
    var error=this.error();
    lib.PQclear(res);
    throw Error(error);
  }
}

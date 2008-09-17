function(res, psql) {
  res.finalize=lib.PQclear;
  this.psql=psql;

  this.res=res;
  this.names=[];
  this.types=[];
  this.formats=[];
  this.length=lib.PQntuples(this.res);
  var nfields=lib.PQnfields(this.res);

  for (var i=0; i<nfields; i++) {
    this.names.push(lib.PQfname(this.res, i).string());
    this.formats.push(lib.PQfformat(this.res, i));
    var	fmt=$parent.decodeUTF8;
    var type=lib.PQftype(this.res, i);
    switch(type) {
      case 16:
	fmt=Boolean;
	break;
      case 18:
      case 19:
      case 1043: // varchar
	break;
      case 20:
      case 21:
      case 22:
      case 23:
      case 700:
      case 701:
      case 703:
	fmt=Number;
	break;
      case 142:
	fmt=function(val) {
	  return XML($parent.decodeUTF8(val));
	}
	break;
      case 17: // bytea
	fmt=function(val) {
	    return new $parent.StringFile(val.replace(/\\([0-9]{3,3})/g,function(m,n) {
	      return String.fromCharCode(parseInt(n,8))
		    }).replace(/\\\\/g,"\\"));
	}
	break;
      case 1184: // timestamptz
	fmt=function(val) {
	  var date=val.match(/([0-9]*)-([0-9]*)-([0-9]*) ([0-9]*):([0-9]*):([0-9]*)(\.([0-9]*))?([+\-][0-9]*)(:(0-9)*)?/);
	  return new Date(Date.UTC(date[1],date[2]-1,date[3],date[4],date[5],date[6],date[7]?Number("0."+date[8])*1000:0)-
			  Number(date[9])*3600000 - (date[10]?Number(date[11])*60000:0));
	}
	break;
      case 1114: // timestamp
	fmt=function(val) {
	  var date=val.match(/([0-9]*)-([0-9]*)-([0-9]*) ([0-9]*):([0-9]*):([0-9]*)(\.([0-9]*))?/);
	  return new Date(date[1],date[2]-1,date[3],date[4],date[5],date[6],date[7]?Number("0."+date[8])*1000:0);
	}
	break;
      case 1082: // date
	fmt=function(val) {
	  var date=val.match(/([0-9]*)-([0-9]*)-([0-9]*)/);
	  return new Date(date[1],date[2],date[3]);
	}
	break;
      case 1083: // time
// 	fmt=function(val) {
// 	  var date=val.match(/([0-9]*):([0-9]*):([0-9]*)\.([0-9]*)/);
// 	  return new Date(new Date(1970,0,2,date[1],date[2],date[3],Number("0."+date[4])*1000).valueOf()%(24*3600000));
// 	}
// 	break;
      case 1266: // timetz
// 	fmt=function(val) {
// 	  var date=val.match(/([0-9]*):([0-9]*):([0-9]*)\.([0-9]*)([+\-][0-9]*)/);
// 	  print(Date.UTC(1970,0,2,date[1],date[2],date[3],Number("0."+date[4])*1000),"\n");
// 	  return new Date((Date.UTC(1970,0,2,date[1],date[2],date[3],Number("0."+date[4])*1000)-
// 			   Number(date[5])*3600000)%(24*3600000));
// 	}
	break;
    }
    this.types.push(fmt);

  }

  this.pos=0;
}

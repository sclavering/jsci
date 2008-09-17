  /*
        stat(path) -> obj

    Perform a stat system call on the given path. Times are returned as date objects.
  */

(function() {

  var stat=function(path) {
    var ret=Pointer(clib['struct stat']);

    if (clib.call_stat(path, ret)==-1) {
      return null;
    }
    return arguments.callee.unistat(ret);
  }

  stat.unistat=function(buf) {
    var i;
    var j;
    var s=buf.$;
    var r={};
    for (i in s) {
      j=i.substr(3);
      switch (i) {
      case "st_atim":
      case "st_ctim":
      case "st_mtim":
	r[j+"e"]=new Date(s[i].tv_sec*1000);
	break;
      case "st_dev":
      case "st_rdev":
	r[j]=s[i].__val;
	break;
      default:
	r[j]=s[i];
      }
    }
    return r;
  }

  return stat;

})()

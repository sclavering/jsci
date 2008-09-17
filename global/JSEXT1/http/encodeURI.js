  /*
        str = http.encodeURI(obj)
    
    Does the precise opposite of [[$curdir.decodeURI]]
  */

  function(obj) {
    var ret="";
    if (obj.protocol!=undefined) ret+=obj.protocol+"://";
    if (obj.username!=undefined) ret+=obj.username;
    if (obj.password!=undefined) ret+=":"+obj.password;
    if (obj.username!=undefined || obj.password!=undefined) ret+="@";
    if (obj.host!=undefined) ret+=obj.host;
    if (obj.port!=undefined) ret+=":"+obj.port;
    if (obj.path!=undefined) ret+=obj.path;
    if (obj.qry!=undefined) ret+="?"+this.encodeQry(obj.qry);
    if (obj.section!=undefined) ret+="#"+this.encode(obj.section);
    return ret;
  }


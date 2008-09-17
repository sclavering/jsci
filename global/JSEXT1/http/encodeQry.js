  /*
        str = http.encodeQry(obj)
    
    Does the precise opposite of [[$curdir.decodeQry]]
  */

  function(obj) {
    if (obj===undefined) return;

    if (obj instanceof Array) {
	var ret=$parent.encodeJSON(obj);
	return ret.substr(1,ret.length-2);
    }
    var ret=[];
    var i;
    for (i in obj) {
      if (obj[i]===null)
	ret.push(encode(i));
      else
        ret.push(encode(i)+"="+encode(String(obj[i])));
    }
    return ret.join("&");
  }


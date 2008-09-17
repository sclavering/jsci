  /*
    Decodes a query string.

        obj = http.decodeQry(str)
    
    _str_ should be the part of an url to the right of the question mark, i.e.
    
        foo=a&two&bar=b
    
    Will return an object containing the properties

        {
          foo:"a",
          two:null,
          bar:"b"
        }

    Query strings not matching this pattern are decoded as if they were
    the innards of JSON brackets, i.e.

        1,2,"three",[4]

    Will return the array

        [1,2,"three",[4]]
  */

  function(qry) {
    if (qry==undefined)
      return;
    try {
      return $parent.decodeJSON("["+decode(qry)+"]");      
    } catch(x) {
      // normal a=x&b=y format

      var get={};
      var vars=qry.split("&");
      var i;
      for (i=0; i<vars.length; i++) {
	var pair=decode(vars[i]).match(/([^=\[\]]*)(\[\])?(=([^]*))?/);
	if (pair[2]) { // array[] parameter
	  if (!get[pair[1]])
	    get[pair[1]]=[];
	  get[pair[1]].push(pair[4]);
	} else {
	  get[pair[1]]=pair[4];
	}
      }
      return get;
    }
  }


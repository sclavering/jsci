/*
         xml = encode(name, val)

     Converts a JavaScript value to an XML node named _name_.
*/

function(name, val) {
  switch(typeof val) {

    case 'object':

      if (val instanceof Date) {

	return <{name}>{String(val)}</{name}>

      } else if (val instanceof Array) {

	var ret=new XMLList;
	for (var i=0; i<val.length; i++)
	  ret += arguments.callee(name, val[i]);
	return ret;

      } else {

        var ret=new XMLList;
        for (var i in val) {
	  ret += arguments.callee(i,val[i]);
	}

	return <{name}>{ret}</{name}>;
      }

    case 'undefined':
      return <{name}></{name}>

    default:
      return <{name}>{String(val)}</{name}>


  }
}

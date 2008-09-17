/*
      html = prettyJS(val)

  Pretty-prints a JavaScript value in HTML.
 */

function(val) {

  var recursion=[];
  if (typeof val == "object" && val !==null)
    return recurse(val, 0);
  else
    return inner(val, 0);

  function recurse(val, level) {
    for (var i=0; i<recursion.length; i++)
      if (recursion[i]===val)
	return level?["RECURSION"]:"RECURSION";

    recursion.push(val);
    var ret=inner(val, level);
    recursion.pop();
    return ret;
  }

  function inner(val, level) {

    var ret;
    var headers;
    
    switch(typeof val) {
    case 'xml':
      var o = XML.prettyPrinting;
      XML.prettyPrinting = true;
      val = String(val);
      XML.prettyPrinting = o;
      // fall through
    case 'string':
      ret=$curdir.escape(val).replace(/\n/g,"<br>\n");
      break;
    case 'number':
    case 'boolean':
    case 'undefined':
      ret=String(val);
      break;
    case 'object':
      if (val===null) {
	ret="null";
	break;
      } else if (val instanceof Array) {
	switch (level) {
	case 0:
	  var row=[];
	  for (var i=0; i<val.length; i++) {
	    var r=recurse(val[i], 1);
	    if (r.length==2) {
	      if (r[0]==headers)
		r.shift();
	      else
		headers=r[0];
	    }
	    row.push("<tr>"+r.join("</tr>\n<tr>")+"</tr>\n");
	  }
	  return "<table>\n"+row.join("")+"</table>";
	case 1:
	  var col=[];
	  for (var i=0; i<val.length; i++) {
	    col.push(recurse(val[i], 0));
	  }
	  return ["<td>"+col.join("</td><td>")+"</td>"];
	}
      } else if (val instanceof Date) {
	  ret=$parent.sprintf("%04d/%02d/%02d %02d:%02d:%02d",
	      val.getFullYear(),
	      val.getMonth()+1,
	      val.getDate(),
	      val.getHours(),
	      val.getMinutes(),
	      val.getSeconds())
	break;
      } else if (val instanceof Object) {
	var keys=[];
	var values=[];
	for (var i in val) {
	  if (val.hasOwnProperty(i)) {
	    keys.push(i);
	    values.push(recurse(val[i], 1-level));
	  }
	}
	
	switch (level) {
	case 0:
	  var row=[];
	  for (var i=0; i<keys.length; i++) {
	    if (values[i].length==2) {
	      if (values[i][0]==headers)
		values[i].shift();
	      else
		headers=values[i][0];
	    }
	    if (values[i].length==2)
	      row.push("<tr><td> </td>"+headers+"</tr>\n");
	  row.push("<th>"+keys[i]+"</th>"+values[i][values[i].length-1]+"</tr>\n");
	  }
	  return "<table>\n"+row.join("")+"</table>";
	case 1:
	  return ["<th>"+keys.join("</th><th>")+"</th>",
		  "<td>"+values.join("</td><td>")+"</td>"];
	}
      }
    }
    switch (level) {
    case 0:
      return ret;
    case 1:
      return ["<td>"+ret+"</td>"];
    }
    
  }
}

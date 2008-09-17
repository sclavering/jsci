/*
      xml = toXML(str)

  Converts a string to xml using the [[XML]] constructor.
  If the string begins with a &lt;? ... ?&gt; line, it
  is removed. If that line includes a UTF 8 encoding
  declaration, the remainder of the string is decoded
  according to UTF 8 before being passed to the
  [[XML]] constructor.

 */

function(str) {
  if (typeof str=="xml")
    return str;

  if (str.substr(0,2)!="<?")
    return XML(str);

  var commend=str.indexOf("?>");

  var intro=XML("<"+str.substr(2,commend-2)+"/>");
  if (intro.@encoding=="UTF-8")
    return XML($parent.decodeUTF8(str.substr(commend+2)));
  else
    return XML(str.substr(commend+2));
    
}

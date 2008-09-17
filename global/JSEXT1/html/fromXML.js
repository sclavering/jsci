/*
  Converts an xml document into a string. Removes closing
  tags for empty tags.
 */

(function(){

  return function(xml) {
    var html=xml.toXMLString();
    html=html.replace(/(<([a-zA-Z0-9]+)[^>]*)\/>/,function(str, all, tag) {
	if (emptyTags[tag])
	  return all+">";
	else
	  return all+"></"+tag+">";
	});
    return html;
	
  }

})()

/*

    array = listdir(doc)


Tries to parse an html directory listing and extract the file names it links to.


### Return value ###

An array containing strings

*/

function(doc) {

  var l=links(doc);
  var ret=[];
  for (var i in l) {
    if (l[i].match(/^[^\/?]*[^?]$/))
      ret.push(l[i]);
  }

  return ret;

}

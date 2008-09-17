/*

    array = links(str)

Returns all urls pointed to by href elements in the document

### Return value ###

An array containing strings

*/

function(doc) {

  var links=doc.match(/<a [^>]*>/gi);
  var ret=[];

  if (links)
    for (var i=0; i<links.length; i++) {
      var href=links[i].match(/href=("([^"]*)"|'([^']*)'|([^ \t>]*))/i);
      if (!href)
        continue;
      ret.push(href[4] || href[3] || href[2]);
    }

  return ret;

}

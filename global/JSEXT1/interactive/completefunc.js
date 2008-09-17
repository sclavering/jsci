/*
  Used by [[$curdir]] to find possible completions to words
  typed in the console.

 */

    function (word, root) {
      try {
	var parts=word.split(".");
	var cur=root || (function(){return this;})(); // global;
	for (var i=0; i<parts.length-1; i++) {
	  if (cur[parts[i]])
	    cur=cur[parts[i]];
	  else
	    return;
	}
	
	var ret=[];
	var lastword=parts[parts.length-1];
	var firstwords=word.substr(0,word.length-lastword.length);
	for (var i in cur) {
	  if (i.substr(0,lastword.length)==lastword)
	    ret.push(firstwords+i);
	}
	return ret;
      } catch(x) {}
    }


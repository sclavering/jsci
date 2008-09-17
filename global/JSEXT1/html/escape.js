/*
      str2 = escape(str)

  Replaces tags with html entities.
 */

(function(){

  var repl={
    '>': '&gt;',
    '<': '&lt;',
    '&': '&amp;'
  };


  return function(str) {
    if (str)
      return str.replace(/[<>&]/g,function(chr) {
	  return repl[chr];
	});
    return "";
  }
 })()

function(fragment) {
  var getters = "";
  for(var i in fragment) getters += 'obj.__defineGetter__(' + uneval(i) + ', function() { return getter_helper.call(obj, ' + uneval(i) + ', ' + uneval(fragment[i]) + '); });\n';

  return "\n\
(function(){ \n\
 \n\
function getter_helper(key, code) { \n\
  delete obj[key]; \n\
  return obj[key] = eval(code); \n\
} \n\
 \n\
const obj = {}; \n\
" + getters + " \n\
return obj; \n\
})()\n";
}

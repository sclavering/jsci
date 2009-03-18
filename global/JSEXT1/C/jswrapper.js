function(fragment) {
  var code = "\n\
    (function(){\n\
    const src = " + uneval(fragment) + "\n\
    const obj = {};\n\
    ";
  for(var i in fragment) code += 'obj.__defineGetter__(' + uneval(i) + ', function() { return getter_helper.call(obj, ' + uneval(i) + '); });\n';
  code += uneval(getter_helper) + "\
    return obj;\
    })()";
  return code;

  // becomes part of the .jswrapper
  function getter_helper(key) {
//     print("getting: " + key + "\n");
    delete obj[key];
//     print("getter " + src[key] + "\n");
    return obj[key] = eval(src[key]);
  }
}

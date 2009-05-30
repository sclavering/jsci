(function() {

return function(filename) {
  return jswrapper(fragment(filename));
};


function runcpp(filename) {
  /*
  cpp is assumed to be the one from GCC.  Tested using "cpp (GCC) 4.2.4 (Ubuntu 4.2.4-1ubuntu3)"
  
  -dD
    keeps "#define FOO 42" and similar, so we can include such constants in the .jswrapper
    expands macros, so that function prototypes are in the form we can parse
  -undef stops cpp leaving __extension__ crap in libmysql.h's output (a gcc-ism)
  --std=gnu99 is just to try and get clib.h's output closer to what our old libcpp was doing
  */
  return JSEXT1.File.read("/usr/bin/cpp '" + filename + "' -dD -undef --std=gnu99 |");
}
/*
Note: the old libcpp hard-coded some standard .h files:

  float.h: empty
  stddef.h:
		 #ifndef _STDDEF_H
		 #define _STDDEF_H
		 typedef unsigned long size_t;
		 typedef short wchar_t;
		 typedef int ptrdiff_t;
     #define inline _inline
     #define NULL 0
		 #endif
   stdarg.h:
		 #ifndef _STDARG_H
		 #define _STDARG_H
		 typedef __builtin_va_list __gnuc_va_list;
		 typedef __builtin_va_list va_list;
		 #endif

Since switching to using GCC's cpp, we lack clib.va_list, and have loads of constants from float.h
*/


function fragment(filename) {
  const obj = jsxcore.cToXML(runcpp(filename));
  var parsed = parse(new XML(obj.xml));
  var src = {};

  for(var i in parsed.structs_and_unions)
    if(parsed.exported_symbols[i])
      src[i] = parsed.structs_and_unions[i];

  for(var i in parsed.sym) {
    if(parsed.exported_symbols[i]) {
      if(src[i]) {
        src[i] = "this['" + i + "']=" + src[i] + ";" + parsed.sym[i];
      } else {
        src[i] = parsed.sym[i];
      }
    }
  }

  return src;
}


function jswrapper(fragment) {
  var getters = "";
  for(var i in fragment) {
    var code = fragment[i];
    if(/^(?:\d+|null|undefined)$/.test(code)) {
      getters += 'obj[' + uneval(i) + '] = ' + code + ';\n';
    } else {
      getters += 'obj.__defineGetter__(' + uneval(i) + ', function() { return getter_helper.call(obj, ' + uneval(i) + ', ' + uneval(fragment[i]) + '); });\n';
    }
  }

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
};

})()

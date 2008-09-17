/*

           sprintf(format, ...)

       Same as [[$curdir.printf]], but returns the string instead of printing it.
*/

(function() {

  function prepargs(format) {
    var outer_args=arguments;
    var i=1;

    var inner_args=[];
    var regex=/%[#0\- +\'I]*([1-9][0-9]*)?(\.[0-9]*)?(hh|h|l|ll|L|q|j|z|t)?([diouxXeEfFgGaAcsCSpn])/g;
    format.replace(regex, eacharg);

    return inner_args;

    function eacharg(fmt, a, b, c, d) {
      switch(d) {
      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
	inner_args.push(Type.int);
	break;
      case 'e':
      case 'E':
      case 'f':
      case 'F':
      case 'g':
      case 'G':
      case 'a':
      case 'A':
	inner_args.push(Type.double);
	break;
      case 'c':
      case 'C':
	inner_args.push(Type.char);
	break;
      case 's':
      case 'S':
	inner_args.push(Type.pointer(Type.char));
	break;
      case 'p':
	inner_args.push(Type.pointer(Type['void']));
	break;
      case 'n':
	inner_args.push(Type.pointer(Type.int));
	break;
      }
      inner_args.push(outer_args[i++]);
    }

  }


  if (!JSEXT_config._WIN32) {
    return function(format) {
      var inner_args=[null, 0, format].concat(prepargs.apply(null,arguments));
      var len=clib.snprintf.apply(null,inner_args);
      var buf=Pointer.malloc(len+1);
      inner_args[0]=buf;
      inner_args[1]=len+1;
      clib.snprintf.apply(null,inner_args);
      return buf.string(len);
    }
  } else {
    return function(format) {
      var inner_args=[null, 0, format].concat(prepargs.apply(null,arguments));
      var trylen=512;
      for (;;) {
	var buf=Pointer.malloc(trylen+1);
	inner_args[0]=buf;
	inner_args[1]=trylen+1;
	var len=clib._snprintf.apply(null,inner_args);
	if (len<trylen) break;
	trylen*=2;
      }
      return buf.string(len);
    }
  }

})()

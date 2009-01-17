/*
      ret = md5( str [, binary=false] )

  Calculates the md5 hash code for the string.

  ### Arguments ###

  * _str_: A string or a file-like object of any size
  * _binary_: Set to true to return a 16 character string
    containing character codes corresponding to the hash value.
    Set to false to return a base16-encoded string, 32 characters long.

  ### Return value ###

  A string

 */

function(str, binary) {
  var md5=arguments.callee.md5;
  var output=Array(16);

  if (typeof(str)=="object" && typeof(str.read)=="function") { // A file-like object

    var ctx=new Pointer(md5.md5_context);
    md5.md5_starts(ctx);
    while (!str.eof()) {
      var substr=encodeUTF8(str.read(65536));
      md5.md5_update(ctx, substr, substr.length);
    }

    md5.md5_finish(ctx, output);

  } else {

    str=encodeUTF8(String(str));
    md5.md5(str, str.length, output);

  }

  if (binary)
    return String.fromCharCode.apply(String,output);
  else {
    var ret="";
    for (var i=0; i<output.length; i++) {
      ret+=("0"+output[i].toString(16)).substr(-2);
    }
    return ret;
  }
}

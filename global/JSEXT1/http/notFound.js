/*
      http.notFound()

  Prints a small message explaining the error code 404 Not found
  and changes this.responseLine to "404 Not found".
  The function is called without arguments, but with a _this_ object
  which must contain the following property:

  * _filename_: A string

  The function does not return a value, but prints its result to
  [[stdout]].
 */

function() {
  this.responseLine="404 Not Found";
	
  print('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">\n');
  print('<HTML><HEAD><TITLE>404 Not found</TITLE></HEAD>');
  print('<BODY><H1>Not found</H1>'+this.filename+'</BODY></HTML>');
//clib.puts(String(this.requestURL));
}
      

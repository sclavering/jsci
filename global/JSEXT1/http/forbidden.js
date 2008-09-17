/*
      http.forbidden()

  Prints a small message explaining the error code 403 Forbidden
  and changes this.responseLine to "403 Forbidden".

  The function does not return a value, but prints its result to
  [[stdout]].
 */
function() {
  this.responseLine="403 Forbidden";
	
  print('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">\r\n');
  print('<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>');
  print('<BODY><H1>Forbidden</H1></BODY></HTML>');
}
      

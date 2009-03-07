/*
  This function is called automatically during the initialization of
  JSEXT.

  ### Invoked with a command line ###

  If JSEXT is called with command-line arguments, then
  the first of them should be a .js file. *shell* will
  load that file and interpret it. If it evaluates to a function,
  i.e. if the last statement in the file is a function definition,
  that function will be called with the remaining command-line
  arguments as its arguments.

  The function will be called with a _this_ object containing
  the property

  * _name_: Its own filename, as given on the command line.

  Any return value from the function will be printed on [[stdout]].

  ### Invoked from a web server ###

  When JSEXT is started as an fcgi server, the web server should
  set an environment variable named JSEXT\_FCGI and pass no
  arguments to JSEXT. *shell* will then call [[$curdir.fcgi]] to
  start the fcgi server.

  ### Invoked with no arguments ###

  If JSEXT is started without arguments, an interactive session
  is started by calling [[$curdir.interactive]].
  
*/

function() {
  if(environment.JSEXT_FCGI) {
    return JSEXT1.fcgi();
  }

  if(environment.GATEWAY_INTERFACE) {
    new JSEXT1.CGI();
    return;
  }

  if(arguments.length) { // execute a program
    var progdir=JSEXT1.path(arguments[0]);
    var progfile=JSEXT1.filename(arguments[0]);
    
    var glob={};
    JSEXT1.ActiveDirectory.call(glob, progdir);

    var prog=JSEXT1.activate.js.call(glob, progfile,"");

    if (typeof(prog)==="function" && prog.name==="") {
      var cx={
	name: Array.prototype.shift.apply(arguments)
      };
      
      try {
	var ret=prog.apply(cx, arguments);
	if (ret!==undefined)
	  print(ret);
      } catch(_err) {
	if (_err.fileName && _err.lineNumber)				
	  print('Line ' + _err.lineNumber + ' in ' + _err.fileName + ':');	
	print((_err.message || _err) + '\\n');
	if (_err.stack) {
	  var stack=_err.stack.split('\\n');
	  print(stack.join('\\n')+'\\n');
	}
	print("\n");
      }
    }
    return;
  }

  return JSEXT1.interactive();
}

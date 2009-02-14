/*

    interactive()

Opens a [[$curdir.Console]] to read commands into jsext. This function is usually invoked automatically by [[$curdir.shell]] if no file when no file is specified on the command-line to jsext.

Statements are evaluated as they are entered. The value of statements that
are not terminated by ; (semicolon) is printed on the console.

*/


  function() {

    var normalprompt="jsext> ";
    var contprompt="> ";
    var cons;
    var cmdbuf;
    var execline;

    if(!stdin.isatty()) {
      cons=stdin;
      cons.Options={};
    } else {
      cons=Console.open(
	    {
  		  prompt: normalprompt,
		histfile: environment.HOME+"/.jsext_history",
        complete: arguments.callee.completefunc
      });

      // Make a function using the Function constructor to avoid closure
      // underscore all variables, as they are visible to the interactive environment
			
      cmdbuf="";
      execline=arguments.callee.execline;
      var global=function(){return this;}();

      for (;;) {
	cmdbuf+=cons.readline();

	if (cons.eof()) break;
	if (js.isCompilableUnit(cmdbuf)) {
	  global.$checkdates();
	  execline(cmdbuf);
	  cmdbuf="";
		
	  cons.Options.prompt=normalprompt;
	} else
	  cons.Options.prompt=contprompt;
      }
      cons.close();
      return 0;
    }

  }



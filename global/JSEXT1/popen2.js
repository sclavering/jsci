/*
[stdin, stdout] = popen2(command)

Opens a command and connects one file-like object to stdin and another to stdout.

Note: popen()'s functionality is available the File constructor.
*/
function(command) {
    
    var stdout=[0,0];
    var stdin=[0,0];

    clib.pipe(stdin);
    clib.pipe(stdout);
    
    var pid=clib.fork();
    if (pid==0) {
      // Child process
      clib.close(stdin[1]);
      clib.dup2(stdin[0],clib.STDIN_FILENO);
      clib.close(stdin[0]);
      clib.close(stdout[0]);
      clib.dup2(stdout[1],clib.STDOUT_FILENO);
      clib.close(stdout[1]);
      var args=command.split(/[ \t]+/);
      args.push(null);
      clib.execvp(args[0],args);
      clib.exit();
    }
    
    var proc={};
    proc.pid=pid;
    
    proc.filecount=2;
    
    var ret=[new File(clib.fdopen(stdin[1], "w")),
	     new File(clib.fdopen(stdout[0], "r"))];
    clib.close(stdin[0]);
    clib.close(stdout[1]);
    
    for (i in ret) {
      ret[i].proc=proc;
      ret[i].close=function() {
	if (!this.closed) {
	  clib.fclose(this.fp);
	  this.closed=true;
	  this.proc.filecount--;
	  if (this.proc.filecount==0) {
	    return clib.waitpid(this.proc.pid);
	  }
	}
      }
    }
    return ret;
  }

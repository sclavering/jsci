/*
interactive()

Opens a [[$curdir.Console]] to read commands into jsext. This function is usually invoked automatically by [[$curdir.shell]] if no file when no file is specified on the command-line to jsext.

Statements are evaluated as they are entered. The value of statements that are not terminated by ; (semicolon) is printed on the console.
*/
(function() {

function interactive() {
  var normalprompt = "jsext> ";
  var contprompt = "> ";
  var cons;
  var cmdbuf;

  if(!stdin.isatty()) {
    cons = stdin;
    cons.Options = {};
  } else {
    cons = Console.open({
      prompt: normalprompt,
      histfile: environment.HOME + "/.jsext_history",
      complete: completefunc,
    });

    // Make a function using the Function constructor to avoid closure
    // underscore all variables, as they are visible to the interactive environment
    
    cmdbuf = "";
    var global = function(){ return this; }();

    for(;;) {
      cmdbuf += cons.readline();
      if(cons.eof()) break;
      if(js.isCompilableUnit(cmdbuf)) {
        execline(cmdbuf);
        cmdbuf = "";
        cons.Options.prompt = normalprompt;
      } else {
        cons.Options.prompt = contprompt;
      }
    }
    cons.close();
    return 0;
  }
}

// the traceback stuff fails because there's no function *named* execline on the stack
function execline(line) {
  try {
    var rval = do_eval(line);
    line = line.replace(/^[ \t\n\r]+|[ \t\n\r]+$/g, '');
    var lastchar = line.substr(line.length - 1);
    if(lastchar != ';' && lastchar != '}' && lastchar != '>' && rval !== undefined) print(String(rval)+'\n');
  } catch(err) {
    if(err.fileName && err.lineNumber) print('Line ' + err.lineNumber + ' in ' + err.fileName + ':');
    print((err.message || err) + '\n');
    if(err.stack) {
      var stack = err.stack.split('\n');
      while(!stack.pop().match(/\bdo_eval\b/));
      stack.pop();
      print(stack.join('\n') + '\n');
    }
  }
}


function do_eval($$code) {
  return eval($$code);
}


// Used to find possible completions to words typed in the console.
function completefunc(word, root) {
  try {
    var parts = word.split(".");
    var cur = root || (function(){ return this; })(); // global;
    for(var i = 0; i < parts.length - 1; i++) {
      if(!cur[parts[i]]) return;
      cur = cur[parts[i]];
    }
    
    var ret = [];
    var lastword = parts[parts.length - 1];
    var firstwords = word.substr(0, word.length - lastword.length);
    for(var i in cur) {
      if(i.substr(0, lastword.length) == lastword)
        ret.push(firstwords + i);
    }
    return ret;
  } catch(x) {}
}


return interactive;

})()

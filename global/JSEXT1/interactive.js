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
      complete: arguments.callee.completefunc,
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


const execline = Function("_line", " \
  try { \
    var _rval = eval(_line); \
    _line = _line.replace(/^[ \\t\\n\\r]*/, '').replace(/[ \\t\\n\\r]*$/, ''); \
    var _lastchar = _line.substr(_line.length - 1); \
    if(_lastchar != ';' && _lastchar != '}' && _lastchar != '>' && _rval !== undefined) print(String(_rval)+'\\n'); \
  } catch(_err) { \
    if(_err.fileName && _err.lineNumber) print('Line ' + _err.lineNumber + ' in ' + _err.fileName + ':'); \
    print((_err.message || _err)+'\\n'); \
    if(_err.stack) { \
      var stack = _err.stack.split('\\n'); \
      while(!stack.pop().match(/execline/)); \
      stack.pop(); \
      print(stack.join('\\n') + '\\n'); \
    } \
  } \
");


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

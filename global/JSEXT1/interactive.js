/*
interactive()

Opens a [[$curdir.Console]] to read commands into jsext. This function is usually invoked automatically by [[$curdir.shell]] if no file when no file is specified on the command-line to jsext.

Statements are evaluated as they are entered. The value of statements that are not terminated by ; (semicolon) is printed on the console.
*/
(function() {

function interactive() {
  const normalprompt = "jsx> ";
  const contprompt = ".... ";
  var cons;
  var cmdbuf;

  if(!stdin.isatty()) {
    cons = stdin;
  } else {
    cons = new Console({
      prompt: normalprompt,
      histfile: environment.HOME + "/.jsext_history",
    });

    cmdbuf = "";
    var global = function(){ return this; }();

    for(;;) {
      cmdbuf += cons.readline();
      if(cons.eof()) break;
      if(isCompilableUnit(cmdbuf)) {
        execline(cmdbuf);
        cmdbuf = "";
        cons.prompt = normalprompt;
      } else {
        cons.prompt = contprompt;
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


/*
new Console(args_obj)

Uses GNU readline/history libraries to read lines of text from a console, allowing the use of arrow keys and line-editing.

Optional arguments:
  histfile: the name/path of a file to save the editor history in (e.g. ~/.foo_history)
  prompt: a string like "> " used to indicate the console is expecting input
  completion_function: A function which will be called when the user presses TAB twice.  It should take one string argument and return an array of possible completions to the string.
*/
function Console(args) {
  this._histfile = args.histfile || null;
  this.prompt = args.prompt || "> ";

  if(this._histfile) JSEXT1.libhistory.read_history(String(this._histfile));
  JSEXT1.libreadline.rl_completion_entry_function.$ = iterate_completion;
}


Console.prototype = {
  prompt: "", // currently public
  closed: false,

  write: print,

  // Writes the history to the history file and closes the console.
  close: function() {
    if(this.closed) return;
    if(this._histfile) JSEXT1.libhistory.write_history(String(this._histfile));
    print("\n");
    this.closed = true;
  },

  // Returns true if the console has been closed with .close().
  eof: function() {
    return this.closed;
  },

  // Reads one line of text from the console.
  readline: function() {
    clib.fflush(clib.stdout.$);
    clib.fflush(clib.stderr.$);
    var line = readline(this.prompt);
    if(line === undefined) {
      this.close();
      return;
    }
    if(line != "") JSEXT1.libhistory.add_history(String(line));
    return line;
  }
};


/*
readline(prompt) => line

The function readline() prints a prompt prompt and then reads and returns a single line of text from the user. If prompt is undefined or the empty string, no prompt is displayed.

If readline encounters an EOF while reading the line, and the line is empty at that point, then undefined is returned. Otherwise, the line is ended just as if a newline had been typed.

If you want the user to be able to get at the line later, you must call history.add() to save the line away in a history list of such lines.
*/
function readline(prompt) {
  prompt = prompt ? String(prompt) : null;
  const ret = JSEXT1.libreadline.readline(prompt);
  if(ret === null) return; // encountered EOF
  const str_ret = ret.string();
  clib.free(ret);
  return str_ret;
}


/*
Beware: this function is called from libreadline, and thus cannot see it's scope (this file) while running.  That's why get_completion_list() is defined inside it, and .completion_cache is stored on arguments.callee rather than using a closure variable.

libreadline passes us a word/token to get completions for, and calls us repeatedly to get the full list of results one at a time.  We have to malloc() the return values, and libreadline will free() them.
*/
function iterate_completion(text_ptr, state) {
  const self = arguments.callee;

  if(state == 0) self.completion_cache = get_completion_list(text_ptr.string());
  if(!self.completion_cache || state >= self.completion_cache.length) return null;
  return clib.strdup(String(self.completion_cache[state]));

  // Completes method/field names for expressions like |foo.bar.baz|
  // Doesn't handle e.g.: "foo".<tab><tab> (i.e. tab-completing methods of strings from a literal)
  function get_completion_list(word) {
    try {
      const parts = word.split(".");
      var cur = (function(){ return this; })(); // get the global object
      for(var i = 0; i < parts.length - 1; ++i) {
        if(!(parts[i] in cur)) return;
        cur = cur[parts[i]];
      }

      const ret = [];
      const lastword = parts[parts.length - 1];
      const firstwords = word.substr(0, word.length - lastword.length);
      for(var i in cur) {
        if(i.substr(0, lastword.length) == lastword)
          ret.push(firstwords + i);
      }
      return ret;
    } catch(x) {
    }
  }
}


return interactive;

})()

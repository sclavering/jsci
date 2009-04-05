/*
new Console(args_obj)

Uses GNU readline/history libraries to read lines of text from a console, allowing the use of arrow keys and line-editing.

Optional arguments:
  histfile: the name/path of a file to save the editor history in (e.g. ~/.foo_history)
  prompt: a string like "> " used to indicate the console is expecting input
  completion_function: A function which will be called when the user presses TAB twice.  It should take one string argument and return an array of possible completions to the string.
*/
(function() {

function Console(args) {
  this._histfile = args.histfile || null;
  this.prompt = args.prompt || "> ";
  this._completion_function = args.completion_function || null;

  if(this._histfile) Console.history.read(this._histfile);
  if(this._completion_function) Console.readline.completion_function(this._completion_function);
}



Console.prototype = {
  prompt: "", // currently public
  closed: false,

  write: print,

  // Writes the history to the history file and closes the console.
  close: function() {
    if(this.closed) return;
    if(this._histfile) Console.history.write(this._histfile);
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
    var line = Console.readline(this.prompt);
    if(line === undefined) {
      this.close();
      return;
    }
    if(line != "") Console.history.add(line);
    return line;
  }
};



return Console;

})()

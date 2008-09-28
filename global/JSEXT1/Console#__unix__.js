/*

    new Console( [Options] )

Uses the GNU readline/history library to read lines of text
from a console, allowing the use of arrow keys and line-editing.

### Arguments ###

* _Options_: An optional argument containing the followin optional options:

  * _histfile_: The name of the file to save the editor history in.
  * _complete_: A function which will be called when the user presses
    TAB+TAB.

### Completion function ###

If a completion function is given, it should take one string argument
and return an array of possible completions to the string.

*/

function(Options) {

  if (Options) {
    for (var i in this.Options)
      if (!Options[i])
	Options[i] = this.Options[i];
    this.Options=Options;
  }

  var Console=arguments.callee;

  if (this.Options.histfile)
    Console.history.read(this.Options.histfile);
  
  if (this.Options.complete)
    Console.readline.completion_function(this.Options.complete);
      
}



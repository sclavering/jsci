/*
  Writes the history to the history file and closes the console.
 */

function() {
  if (!this.closed) {
    if (this.Options.histfile)
      $parent.history.write(this.Options.histfile);
    print("\n");
    this.closed=true;
  }
}


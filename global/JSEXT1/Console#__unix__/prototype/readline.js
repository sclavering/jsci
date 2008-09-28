/*
      str = cons.readline()

  Reads one line of text from the console.

 */
function() {
  clib.fflush(clib.stdout.$);
  clib.fflush(clib.stderr.$);
  var line=$parent.readline(this.Options.prompt);
  if (line===undefined) {
    this.close();
    return;
  }
  if (line!="")
    $parent.history.add(line);
  return line;
}


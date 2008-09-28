function() {
  this.write(this.Options.prompt);
  this.Frame.EmptyQueue(); // empty message queue

  with (this.Edit) {
    var pos=GETSEL()[0];
    var lineno=LINEFROMCHAR(pos);
    this.promptlen[lineno]=pos-LINEINDEX(lineno);
  }

  this.newLine=undefined;
  this.Edit.listening=true;
  while (this.newLine==undefined) {
    if (this.Frame.OneLoop()!==undefined) {
      // main loop stopped
      // probably because application quit
      this.closed=true;
      return;
    }
  }
  
  return this.newLine;
}


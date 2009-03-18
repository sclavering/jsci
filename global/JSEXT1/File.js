/*
new File(filename [, mode])
new File(FILE *)

Opens an existing file or creates a new one. The second form of the constructor can be used when a library function returns a FILE * which you want to encapsulate in a file object.

The file is closed when the file object is garbage collected, or when [[.close]] is called, whichever comes first.

### Opening processes ###

Filenames that end in | are interpreted as commands that are to be executed. The standard output of these commands can be read through the newly created File object.

yes = new File("yes |");

Filenames that begin with | are interpreted as commands that are to be executed. The standard input of these commands can be witten through the newly created File object.

letter = new File("| mail");

To read and write to a process, use [[$curdir.pipe2]].
*/

function(filename, mode) {

  function popen(command) {
    if(mode != "r" && mode != "w" && mode != "a") throw("popen: Invalid mode");
    fp = clib.popen(command, mode);
    if(!fp) throw new Error("File: Coupld not open process '" + command + "'");
    fp.finalize = clib.pclose;
    this.name = "<command " + command + ">";
    this.close = pclose;
    this.fp = fp;
  }

  function pclose() {
    if(!this.closed) {
      var ret = clib.pclose(this.fp);
      this.fp.finalize = null;
      this.closed = true;
      if(ret >= 256) ret /= 256;
      return ret;
    }
  }

  var fp;
  if(arguments.length > 0) {
    if(typeof filename == "string") {
      if(filename[0] == '|') {
        if(filename[filename.length - 1] == '|') {
          // | and |.
          throw new Error("Use popen2 for filters");
        } else {
          // | only.
          if(mode === undefined) mode="w";
          popen.call(this, filename.substr(1));
        }
      } else if(filename[filename.length - 1] == '|') {
        // only |
        if(mode === undefined) mode="r";
        popen.call(this, filename.substr(0, filename.length - 1));
      } else {
        // file
        if(mode === undefined) mode="r";
        fp = clib.fopen(filename, mode);
        if(fp == null) throw new Error("File: Could not open file '" + filename + "'");
        fp.finalize = clib.fclose;
        this.name = filename;
        this.fp = fp;
      }

    } else {
      // filename is a pointer
      this.fp = filename;
      this.name = "<unknown>";
    }

    this.closed = false;
  }
}

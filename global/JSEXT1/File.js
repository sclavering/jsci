(function() {

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
*/
function File(filename, mode) {

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



File.prototype = {
  close: function() {
    if(this.closed) return;
    clib.fclose(this.fp);
    this.closed = true;
    this.fp.finalize = null;
  },


  /*
  file.eof()

  Returns true if end-of-file has been reached.
  */
  eof: function() {
    return clib.feof(this.fp) || clib.ferror(this.fp) ? true : false;
  },


  /*
  num = file.fileno()

  Returns a number, the operating system's file number for the underlying file.
  */
  fileno: function() {
    return clib.fileno(this.fp);
  },


  /*
  file.flush()

  Flushes buffer from c library to operating system
  */
  flush: function() {
    if(clib.fflush(this.fp) != 0) throw new Error(os.error('write'));
  },


  /*
  str = file.read(n)

  Reads _n_ bytes or until end of file, whichever comes first. Returns result as a string.

  str = file.read()

  Reads an entire file (from current position to end of file) and returns it as a string      
  */
  read: function(size) {
    size = Number(size || -1);
    if(size == 0) return "";
    if(size < 0) {
      var trysize = 4096;
      var ret = "";
      while(!this.eof()) ret += this.read(trysize);
      return ret;
    }
    var buf = Pointer.malloc(size);
    var len = clib.fread(buf, 1, size, this.fp);
    return buf.string(len);
  },
  

  /*
  file.seek (offset)

  Moves file pointer to the given position.
  */
  seek: function(offset) {
    clib.clearerr(this.fp);
    clib.fseek(this.fp, offset, clib.SEEK_SET);
  },


  /*
  file.write(str)

  Writes _str_ to file at the current position.  The argument is converted to a [[String]] before being written.
  */
  write: function(str) {
    str = String(str);
    if(!str) return;
    if(clib.fwrite(str, 1, str.length, this.fp) == 0) throw new Error(os.error('write'));
  },
};



/*
str = File.read(path)

Helper function to read the entire contents of a file.
*/
File.read = function(path) {
  const f = new File(path);
  const str = f.read();
  f.close();
  return str;
};



File.write = function(path, contents) {
  const f = new File(path, 'w');
  f.write(contents);
  f.close();
};



return File;

})()

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

To read and write to a process, use [[$curdir.pipe2]].
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
  bool = file.isatty()

  Returns true if the underlying file is a terminal.    
  */
  isatty: function() {
    return clib.isatty(this.fileno());
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
  file.readline([size])

  Returns one line of text, including terminating newline character.
  If a _size_ is given, this will be the maximal line length.

  Not Binary-safe if operating system's fgets reads null characters
  */
  readline: function(size) {
    if(arguments.length < 1) size = -1;
    if(size < 0) {
      var trysize = 4096;
      var ret = "";
      while(!this.eof()) {
        ret += this.readline(trysize);
        if(ret[ret.length - 1] == '\n') break;
      }
      return ret;
    }
    var buf = Pointer.malloc(size + 1);
    var line = clib.fgets(buf, size + 1, this.fp);
    return buf.string();
  },


  /*
  array = file.readlines([size])

  Returns an array of lines with terminating newlines removed.
  If a _size_ argument is given, this is the maximal line length.
  */
  readlines: function(size) {
    if(arguments.length < 1) size = -1;
    if(size < 0) {
      var ret = [];
      while(!this.eof()) ret.push(this.readline());
      return ret;
    }
    var buf = this.read(size);
    return buf.split(/\n/);
  },
  

  /*
  file.seek (offset)

  Moves file pointer to the given position.
  See also [[$curdir.seekEnd]] and [[$curdir.seekRel]].
  */
  seek: function(offset) {
    clib.clearerr(this.fp);
    clib.fseek(this.fp, offset, clib.SEEK_SET);
  },
  

  /*
  file.seekEnd (offset)
    
  Moves file pointer to the given position, relative to the end of the file. _offset_ will generally be zero or a negative number.

  See also [[$curdir.seek]] and [[$curdir.seekRel]].
  */
  seekEnd: function(offset) {
    clib.clearerr(this.fp);
    clib.fseek(this.fp, offset, clib.SEEK_END);
  },
  

  /*
  file.seekRel (offset)
    
  Moves file pointer to the given position, relative to the current position.

  See also [[seek]] and [[seekEnd]].
  */
  seekRel: function(offset) {
    clib.clearerr(this.fp);
    clib.fseek(this.fp, offset, clib.SEEK_CUR);
  },
  

  /*
  file.stat() -> obj
    
  Returns an object with vital stats from a file's inode.
  */
  stat: function() {
    var ret = Pointer(clib['struct stat']);
    if(clib.call_fstat(this.fileno(), ret) == -1) return null;
    return $parent.$parent.stat.unistat(ret);
  },


  /*
  num = file.tell()

  Returns the current file position.
  */
  tell: function() {
    return clib.ftell(this.fp);
  },


  /*
  file.truncate()

  Truncates the file at its current position, i.e. removes the rest of the file and reduces the file's size to the current file position.
  */
  truncate: function(size) {
    clib.ftruncate(this.fileno(), size);
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
file = File.tmp() 
       
Return a new file object opened in update mode ("w+"). The file has no directory entries associated with it and will be automatically deleted once there are no file descriptors for the file. Availability: Unix, Windows. 
*/
File.tmp = function() {
  return new File(clib.tmpfile());
};



return File;

})()

    /*
          file.write (str)

      Writes _str_ to file at the current position.
      The argument is converted to a [[String]] before being
      written.
    */
    
    function(str) {
      str=String(str);
      if (str=="")
        return;
      if (clib.fwrite(str, 1, str.length, this.fp)==0)
        throw new Error(os.error('write'));
    }


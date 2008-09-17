    /*
      
          file.flush()

      Flushes buffer from c library to operating system

    */

    function() {
      if (clib.fflush(this.fp)!=0)
        throw new Error(os.error('write'));
    }


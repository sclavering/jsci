    /*
          file.stat() -> obj
      
      Returns an object with vital stats from a file's inode.

    */

    function() {
      var ret=Pointer(clib['struct stat']);
      if (clib.call_fstat(this.fileno(), ret)==-1) {
	return null;
      }
      return $parent.$parent.stat.unistat(ret);
    }


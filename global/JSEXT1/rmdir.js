  /*
       rmdir(path [, recursive=false])
    
   The directory is only removed if it
   is empty.

   If _recursive_ is true and the leaf directory is
   successfully removed, directories corresponding to rightmost path
   segments will be pruned way until either the whole path is
   consumed or an error occurs.  Errors during this latter phase are
   ignored -- they generally mean that a directory was not empty.

  */

  function(name, recursive) {
    if (!recursive) {
      if (clib.rmdir(name)==-1)
	throw new Error(os.error("dir.rm"));
    }
      
    for(;;) {
      if (clib.rmdir(name)==-1)
	break;
      var slashpos = name.lastIndexOf('/');
      if (slashpos==-1) break;
      name=name.substr(0,slashpos);
    }
  }


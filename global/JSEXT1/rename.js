  /*
       rename(old, new [, recursive = false])

   Super-rename; create directories as necessary and delete any left
   empty.  Works like rename, except creation of any intermediate
   directories needed to make the new pathname good is attempted
   first.  After the rename, directories corresponding to rightmost
   path segments of the old name will be pruned way until either the
   whole path is consumed or a nonempty directory is found.
    
   Note: this function can fail with the new directory structure made
   if you lack permissions needed to unlink the leaf directory or
   file.
     */

  function(oldpath, newpath, recursive) {
    if (!recursive) {
      if (clib.rename(oldpath, newpath))
	throw new Error(os.error("rename"));
      return;
    }

    var lastslash = newpath.lastIndexOf('/');
    if (lastslash!=-1) {
      mkdir(newpath.substr(0,lastslash), true);
    }

    arguments.callee(oldpath,newpath);

    var lastslash = oldpath.lastIndexOf('/');
    if (lastslash!=-1) {
      rmdir(oldpath.substr(0,lastslash), true);
    }
    
  }


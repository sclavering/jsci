  /*
          array = unlink(path [, recursive = false])

      Deletes a file or a tree,
      returning an array with those files and directories
      that could not be removed
     */

  function(path, recursive) {

    if (!recursive) {
      if (clib.unlink(path)==-1)
	throw new Error(os.error("File.rm '"+path+"'"));
      return;
    }

    var errors=[];
    if (path[path.length-1]==JSEXT_config.sep)
      path=path.substr(0,path.length-1);
    recursive(path);
    return errors;

    function recursive(path) {
      if (isdir(path)) {
	var d=dir(path);
	for (var i in d) {
	  recursive(path+JSEXT_config.sep+d[i]);
	}
	if (clib.rmdir(path)==-1)
	  errors.push(path);
      } else {
	if (clib.unlink(path)==-1)
	  errors.push(path);
      }
    }
  }


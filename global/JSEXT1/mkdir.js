  /*
       mkdir(path [, mode=0777] [, recursive=false])

   Create an empty directory.

   If _recursive_ is _true_, any intermediate path segment (not
   just the rightmost) will be created if it does not exist.

     */

  function(path) {
    var mode=0777;
    var recursive=false;
    for (var i=1; i<arguments.length; i++) {
      switch(typeof arguments[i]) {
      case 'boolean':
	recursive=arguments[i];
        break;
      case 'number':
	mode=arguments[i];
	break;
      }
    }


    if (!recursive) {
      if (clib.mkdir(path, mode)) {
	throw new Error(os.error("mkdir"));
      }
      return;
    }
    

    var tomake=[];
    for(;;) {
      if (exists(path)) break;
      var lastslash=path.lastIndexOf(JSEXT_config.sep);
      if (lastslash==-1) {
	tomake.unshift(path);
	path=curdir;
	break;
      }
      tomake.unshift(path.substr(lastslash+1));
      path=path.substr(0,lastslash);
    }
  
    while (tomake.length) {
      var thisdir=tomake.shift();
      clib.mkdir(path+JSEXT_config.sep+thisdir, mode);
      path+=JSEXT_config.sep+thisdir;
    }
  }

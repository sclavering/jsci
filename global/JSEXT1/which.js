  /*
          which(name)

      Searches for _name_ in search path
     */

  function(name) {
    if (name.indexOf(JSEXT_config.sep)!=-1) { // relative or absolute path, don't search
      if (access(name,"x"))
	return name;
      return false;
    }
    var path=environment.PATH.split(JSEXT_config.pathsep);
    for (var p in path) {
      var tryname=path[p]+JSEXT_config.sep+name;
      if (access(tryname,"x"))
	return tryname;
    }
    return false;
  }

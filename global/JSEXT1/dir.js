  /*
        array = dir( [path="."] )

    Return a list containing the names of the entries in the directory.
 
    * _path_: path of directory to list
 
    The list is in arbitrary order.  It does not include the special
    entries '.' and '..' even if they are present in the directory.
  */

  function(path) {
    if (!path)
      path=".";
    var d=clib.opendir(path);
    if (!d)
      throw new Error(os.error("dir "+path));
  
    var ret=[];
  
    for(;;) {
      var e=clib.readdir(d);
      if (e==null) break;
      e=e.$;
      var str=e.d_name;
      var len=str.indexOf("\0");
      if (len!=-1)
	str=str.substr(0,len);
      if (str!="." && str!="..")
	ret.push(str);
    }
  
    clib.closedir(d);
    return ret;
  }

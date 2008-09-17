  /*
        string = tmpdir([prefix])

    Return the name of a newly created, empty directory
  */

  function(prefix) {
    if (prefix===undefined)
      prefix="/tmp/mkdtemp";
    var arg=[];
    var i;
    prefix+="XXXXXX";
    for (i=0; i<prefix.length; i++)
      arg.push(prefix.charCodeAt(i));
    arg.push(0);
    var fd=clib.mkdtemp(arg);
    if (fd===null)
      throw new Error(os.error("tmpdir"));
    arg.pop();
    return String.fromCharCode.apply(String,arg);
  }


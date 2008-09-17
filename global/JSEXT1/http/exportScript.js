/*

     This function packages a JavaScript file together with all its
     dependencies so that a web browser can use it.

         exportScript([refresh=false])

     No parameters are passed to the function, but it must be
     passed a _this_ object which contains the following properties:

     * _clientdir_: An [[$parent.ActiveDirectory]] object representing the root directory.
     * _filename_: A string containing an absolute path, where root is
       the specified host directory, not the filesystem root.

     The file is parsed using [[$parent.js.parse]] to find its
     dependencies, i.e. global variables which are referenced
     in the file but not defined. An attempt is made to resolve those
     variables using the normal jsext search path. No error is raised
     if they can not be resolved. The assumption is that the web browser
     will somehow resolve them. If _filename_ is a subdirectory,
     the functions in the file are
     slightly modified to enclose the function bodies with a _with_
     statement so that other functions in the same directory can be
     referenced without qualifier and at the same time without polluting
     the global object.

     All this processing is cached in the exportScript.scriptCache
     property.

     ### Arguments ###

     * _refresh_: Boolean. If true, empties script cache.

     ### Return value ###
     
     This function does not return a value, but prints its
     output on [[stdout]].

    */

(function() {

var scriptCache={};

return function (refresh) {

  try {
    if (refresh)
      scriptCache={};
    
    var dir=this.clientdir;
    var filename=this.filename;
    
    this.responseHeaders.contentType="application/javascript";
    this.responseHeaders.cacheControl="max-age=3600"; // Allow scripts to be cached for an hour
    
    var pathparts=filename.split(JSEXT_config.sep);
    var curdir=dir;
    for (var i=1; i<pathparts.length-1; i++) {
      curdir=curdir[pathparts[i]];
    }
  
    var onlyFilename=$parent.filename(filename);

    
    var script=scriptCache[this.requestURL];
    var mtime=$parent.stat(curdir.$path+JSEXT_config.sep+onlyFilename).mtime;
    
    if (!script || mtime>script.mtime) {
      var parse=$parent.js.parse($parent.read(curdir.$path+JSEXT_config.sep+onlyFilename));
      merge(parse.p1implied, parse.p2implied);
      script=$parent.js['export'].call(this, parse.p1implied, undefined, curdir);
      // If the script evaluates to an anonymous function, then call it!
      var names=$parent.js.callify(parse.statements);

      var where="";
      varptr=curdir;
      while (varptr.$name) {
	where = '["' + varptr.$name + '"]' + where;
	varptr=varptr.$parent;
      }
      if (where!="") {
	parse.statements=$parent.js.withify(parse.statements, "window"+where);
      }
      script+="\n"+$parent.js.unparse(parse.statements);

      script=new String(script);
      script.mtime=mtime;
      script.names=names;
      scriptCache[this.requestURL]=script;
    }

    if (script.names) {
      var args=getFormDataByNames.call(this, script.names);
      for (var i=0; i<args.length; i++)
	if (args[i]===undefined)
	  args[i]=null;
      print("$JSEXT_args="+$parent.encodeJSON(args)+";\n"+
	    script+"\n"+
	    "if ($JSEXT_tmp!==undefined)\n"+
	    "  document.write($JSEXT_tmp);\n");
    } else
      print(script);
    
  } catch (x) {

    print(' <br/>\n');
    if (x.fileName && x.lineNumber)
      print('Line '+x.lineNumber+' in '+x.fileName+':');
    print(x+' <br/>\n');
    if (x.stack) {
      var stack=x.stack.split('\n');
      if (stack.length>6)
	print(stack.slice(1,stack.length-5).join(' <br/>\n')+' <br/>\n');
    }

  }
  
  function merge(a, b) {
    for (var i in b)
      if (hasOwnProperty.call(b, i)) {
	if (!hasOwnProperty.call(a, i))
	  a[i]=b[i];
	else {
	  merge(a[i],b[i]);
	}
      }
  }
}


})()

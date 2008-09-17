/* 
          export(dep, [predef, [dir]])

      Returns JavaScript source code which can be sent
      to a web browser, defining the JSEXT functions or
      other values specified
      in the _dep_ argument and all functions they depend on.

      The objects will be available to a script running in
      the browser under the same names as a script running on the
      host.

      ### Arguments ###

      * _dep_: An object containing names of dependencies, as
        explained below.
      * _predef_: An object containing names of objects that have
        already been defined in the browser. Has the same format
        as _dep_.
      * _dir_: An ActiveDirectory object representing the JSEXT
        global object.

      The _dep_ and _predef_ objects define object names in a
      recursive manner:

      ---

      **Example**

      ---

      Say you want to export two symbols, a.b.c and a.d.e. The
      _dep_ object should then contain:

          {
	    a: {
	         b: {
		      c: {}
                    }
                 d: {
                      e: {}
                    }
               }
          }

      ---

      If the _dep_ object contains a property or a sub-property with
      the special name *(with)*, including the parens, then _export_
      will search for the properties of *(with)* in two places:
      As children of *(with)*'s parent and as properties of the
      global object.

*/

function(dep, predef, dir) {
  var exp=arguments.callee;

  var global=arguments.callee.global;
  var hasOwnProperty=Object.prototype.hasOwnProperty;

  var sort=[];

  dir = dir || {};
  var root=dir;
  while (root.$parent)
    root=root.$parent;

  if (!root.$url)
    root.$url="/";

  var unresolved=false;

  loaddeps(dep, [dir, global]);

  function loaddeps(deps, scope, skipDollar) {
    //    print(deps.toSource(),"\n");
    //    print("{\n");
    walk(deps, function(path) {

      var obj;

      for (var i=0; i<scope.length; i++)
	if (hasOwnProperty.call(scope[i], path[0])) {
	  var obj=scope[i];
	  break;
	}

      //      if (!obj) return; // unresolved
      // maybe it's the basis for (with), so take a look
      
      //      print("exp "+path+"\n");

      for (var i=0; i<path.length; i++) {
	if (path[i]==='(with)') { // Can go either way - back to root or inside ''with'' object
	  if (obj) // maybe it's with(externallydefined){}
	    scope.unshift(obj);
	  var ret=arguments.callee(path.slice(i+1));
	  if (obj)
	    scope.shift();
	  return ret;
	} else if (!obj || !hasOwnProperty.call(obj, path[i])) { // Continue all the same because we may hit a ''with'' and continue global
	  obj=undefined;
	  continue;
	} else {
	  var parent=obj;
	  obj=obj[path[i]];
	}

	if (obj.$filename || obj.$_url) {
	  if (obj.$reg!==undefined) {
	  } else if (ispredef(path)) {
	  } else {
	    if (!obj.$parent) {
	      obj.$parent=parent;
	      obj.$name=path[i];
	      obj.$notActive=true;
	    }

	    obj.$reg=true;

	    if (obj.$dep) {
	      loaddeps(obj.$dep[0], [obj.$parent, global]);
	    }

	    if (obj.$_url) {
	      loaddeps({"JSEXT1":{"http":{"function":{}}}}, [global]);
	    }

	    obj.$reg=sort.length;
	    sort.push(obj);

	    if (obj.$dep) {
	      loaddeps(obj.$dep[1], [obj.$parent, global]);
	    }
	  }
	}

      }

      if (obj && !skipDollar) {
	// Use obj as the dependency object to load everything. Just omit $...
	return loaddeps(obj, [obj], true);
      }
    }, skipDollar);
    //    print("}\n");
  }

  var ret=[];//"if (!this.$JSEXT_root) this.$JSEXT_root={};"];

  for (var i=0; i<sort.length; i++) {
    if (sort[i]) {
      if (sort[i].$filename)
	package_js(normpath(sort[i]), sort[i].$filename, sort[i].$notActive&&0);
      else if (sort[i].$_url)
	package_jsx(normpath(sort[i]), sort[i].$_url, sort[i].$notActive&&0);
      delete sort[i].$reg;
    }
  }

  return ret.join("\n");

  function ispredef(path) {
    if (!predef)
      return false;
    var ptr=predef;
    for (var i=0; i<path.length; i++) {
      if (!hasOwnProperty.call(ptr, path[i])) {
	// At the end of common path. If predef path continues, then the answer is no.
	for (var i in ptr)
	  if (hasOwnProperty.call(ptr, i))
	    return false;
	return true;
      }
      ptr=ptr[path[i]];
    }
    return true;
  }

  function normpath(obj) {
    var ret=[];
    while (obj.$name) {
      ret.push(obj.$name);
      obj=obj.$parent;
    }
    ret.push("this");
    /*
    if (obj==global)
      ret.push("this");
    else
      ret.push("$JSEXT_root");
    */

    ret.reverse();

    return ret;
  }

  function sortdep(path, obj) {
    if (obj.$fileno)
      sort[obj.$fileno]=obj;
  }

  // walks a dependency tree and calls func for each node.
  // func will be passed one argument: an array containing the search path down to the current node.
  function walk(dep, func, skipDollar, path) {
    if (typeof dep !== "object")
      return;
    path=path || [];
    var leafnode=true;
    for (var i in dep) {
      if (hasOwnProperty.call(dep, i) && (!skipDollar || i[0]!='$')) {
	leafnode=false;
	path.push(i);
	walk(dep[i], func, skipDollar, path);
	path.pop();
      }
    }
    if (leafnode)
      func(path, dep);
  }

  function package_js(parts, filename, notactive) {
    var contents=$parent.read(filename);
    var name=parts.shift();
    ret.push("// "+filename);
    for (var i=0; i<parts.length; i++) {
      var oldname=name;
      name+="['"+parts[i]+"']";
      if (i<parts.length-1) {
	ret.push("if (!"+name+") {",
		 "  "+name+" = {};",
		 "}");
	if (!notactive)
	  ret.push(name+".$parent = "+oldname+";",
		   name+".$name = '"+parts[i]+"';",
		   name+".$curdir = "+name+";"
		   );
      }
    }
    
    ret.push(name+"=(function(){with(this){\n"+contents+"\n}}).call("+oldname+");");
  }

  function package_jsx(parts, url, notactive) {
    var name=parts.shift();
    for (var i=0; i<parts.length; i++) {
      var oldname=name;
      name+="['"+parts[i]+"']";
      if (i<parts.length-1) {
	ret.push("if (!"+name+") {",
		 "  "+name+" = {};",
		 "}");
	if (!notactive)
	  ret.push(name+".$parent = "+oldname+";",
		   name+".$name = '"+parts[i]+"';",
		   name+".$curdir ="+name+";"
		   );
      }
    }
    
    ret.push(name+"=JSEXT1.http['function']('"+url+"');");
  }

}


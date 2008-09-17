({
  js: function(filename, extension) {
    var timestamp={};

    for each (var ext in ['js', 'dep', 'jsm']) {
      var stat=$parent.$parent.stat(this.$path+JSEXT_config.sep+filename+"."+ext);
      timestamp[ext]=stat && stat.mtime;
    }

    var w;
    if (filename!="with")
      w=this['with'];

    if (timestamp.js > timestamp.dep || (w && w.$timestamp > timestamp.dep) ||
	timestamp.js > timestamp.jsm || (w && w.$timestamp > timestamp.jsm)) {
      var js=$parent.$parent.read(this.$path+JSEXT_config.sep+filename+extension);

      var parse=$parent.parse(js, filename);
      // Rename dependencies on arguments.callee in last function

      if (w) {
	var ws=$parent.$parent.read(w.$filename);
	ws=ws.substr(6).
	  replace(/^[ \r\n\t]*\[/,"").
	  replace(/\][; \r\n\t]*$/,"").split(/[ \t\n]*,[ \t\n]*/g);
	
	parse.statements=$parent.withify(parse.statements, "this");
	for (var i=ws.length; i--;) {
	  parse.statements=$parent.withify(parse.statements, ws[i]);
	  var parts=[];
	  var root=ws[i].replace(/\.([^\.\[]*)|(\[\'([^\']*)\'\])|(\[\"([^\"]*)\"\])/g,
				 function(a,b,c,d,e,f) {
				   parts.push(b || d || f);
				   return "";
				 });
	  parts.unshift(root);
	  parts.push("(with)");
	  for (var j=parts.length; j--;) {
	    var obj={};
	    obj[parts[j]]=parse.p1implied;
	    parse.p1implied=obj;
	  }
	}
      }
	  
      var last=parse.statements[parse.statements.length-1];
      if (last.id==="function" && last.func['(name)']===undefined && last.func['(selfimplied)']) {
	var selfdep={};
	selfdep[filename.replace(/#.*/,"")]=last.func['(selfimplied)'];
	merge(parse.p2implied, selfdep);
      }

      $parent.bodify(parse.statements);
      var js=$parent.unparse(parse.statements);


      $parent.$parent.write(this.$path+JSEXT_config.sep+filename+".dep",
		  $parent.$parent.encodeJSON([parse.p1implied, parse.p2implied]));
      $parent.$parent.write(this.$path+JSEXT_config.sep+filename+".jsm",
		  js);

      timestamp.jsm=new Date;
    }

    return {
      $timestamp: timestamp.jsm,
      $filename: this.$path+JSEXT_config.sep+filename+".jsm",
      $dep: $parent.$parent.decodeJSON($parent.$parent.read(this.$path+JSEXT_config.sep+filename+".dep"))
    };

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

  },

  jsx: function(filename, extension) {
    var path=[];
    var curdir=this;
    while (!curdir.$url) {
      path.unshift(curdir.$name+"/")
	curdir=curdir.$parent;
    }
    return {
      $_url: curdir.$url+path.join("")+filename+extension
    };
  }
})

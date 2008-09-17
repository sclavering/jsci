function() {

  var deleteext={jsm: true,
		 xdk: true,
		 so: true,
		 pch: true,
		 exp:true,
		 lib: true,
		 dll: true,
		 obj: true,
		 pdb: true,
		 dep: true};

  recurse(JSEXT1.getcwd());

  function recurse(src) {
    print(src, "\n");
    var d=JSEXT1.dir(src);
    for each (var fn in d) {
	var ex=fn.match(/\.([^.]*)$/);
	if (ex)
	  ex=ex[1];
	if (deleteext[ex] || fn.match(/\~$/) || fn.match(/^#/))
	  JSEXT1.unlink(src+"/"+fn);
	if (JSEXT1.isdir(src+"/"+fn))
	  recurse(src+"/"+fn);
      }
  }
}

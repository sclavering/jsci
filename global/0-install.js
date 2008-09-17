function(dir) {

  var installext={js: true,
		  jsx: true,
		  xdk: true,
		  so: true,
		  pch: true,
		  dep: true,
		  jsm: true};

  recurse(dir, JSEXT1.getcwd());
  JSEXT1.system("install 0-init.js "+dir+"/0-init.js");

  function recurse(dest, src) {
    print(dest, "\n");
    JSEXT1.mkdir(dest, true);
    var d=JSEXT1.dir(src);
    for each (var fn in d)
      if (fn.match(/^[^ -@]/)) {
	var ex=fn.match(/\.([^.]*)$/);
	if (ex)
	  ex=ex[1];
	if (installext[ex])
	  JSEXT1.system("install -p "+src+"/"+fn+" "+dest+"/"+fn);
	if (JSEXT1.isdir(src+"/"+fn))
	  recurse(dest+"/"+fn, src+"/"+fn);
      }
  }
}

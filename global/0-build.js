// XXX broken.  Recurses through global/ and builds .so and .pch (now .jswrapper) files for .c and .h files, so that when jsext is run as an unprivileged user (e.g. nobody, or wwwdata) native code actually works
function() {

  // It is necessary to bypass the activedirectory for the current directory, because
  // it includes an "Object" property, which is not equal to the Object constructor...

  with(this) {

    function processdir(path) {
      print("Processing ",path,"\n");

      var fn=JSEXT1.dir(path);
      
      for (var i=0; i<fn.length; i++) {
        var filename=fn[i];
        if(/^(([^ -@][^\.]*))(\.(.*))?/.test(fn[i])) processfile(path, fn[i]);
      }

      function processfile(path, filename) {

	if(JSEXT1.isdir(path + '/' + filename)) {
	  processdir(path + '/' + filename);
	  return;
	}

	var extension=filename.match(/\.[^.]*$/);
	if (!extension)
	  return;
	extension=extension[0];

	var onlyfilename=filename.substr(0,filename.length-extension.length);
	
	switch(extension) {
	case '.c':
	case '.h':
	  try {
	    JSEXT1.activate.c.call({$path:path},onlyfilename,extension);
	  } catch(x) {
	    print(x,"\n");
	  }
	  break;
	}
      }
    }
    
    processdir(JSEXT1.getcwd());
    
  }
}

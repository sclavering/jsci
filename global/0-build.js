function() {

  // It is necessary to bypass the activedirectory for the current directory, because
  // it includes an "Object" property, which is not equal to the Object constructor...

  with(this) {

    function processdir(path) {
      print("Processing ",path,"\n");

      var fn=JSEXT1.dir(path);
      var jsdir = new JSEXT1.ActiveDirectory(path, JSEXT1.js['export'].handlers);
      
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
	case '.js':
	  try {
	    JSEXT1.js['export'].handlers.js.call(jsdir, onlyfilename, extension);
	  } catch(x) {
	  }
	  break;
	}
      }
    }
    
    processdir(JSEXT1.getcwd());
    
  }
}

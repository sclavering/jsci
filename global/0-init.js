function(args) {
  this.Type = args.Type;
  this.Pointer = args.Pointer;
  this.Dl = args.Dl;
  this.load = args.load;
  this.environment = args.environment;
  this.gc = args.gc;

  // avoid unwanted closures
  var xload = new Function("filename", "return load(filename);");

  xload("0-ffi.js");

  $path=".";
  $curdir=this;

  clib = xload("clib.jswrapper");

  var JSEXT = {
    $parent: this,
    $name: "JSEXT1",
    $path: './JSEXT1',
  };

  JSEXT.$curdir=JSEXT;

  for (var i=0; clib['dl '+i]; i++)
    ;

  this.JSEXT1 = JSEXT;

  const ActiveDirectory = xload('JSEXT1/ActiveDirectory.js');

  // os.js has methods like .getcwd() and .stat() that ActiveDirectory requires 
  JSEXT.os = ActiveDirectory.handlers.js.call(JSEXT, 'os', ".js");

  JSEXT.$path = JSEXT.os.getcwd() + '/JSEXT1';
  JSEXT.ActiveDirectory = ActiveDirectory;

  ActiveDirectory.call(this, JSEXT.os.getcwd());
  ActiveDirectory.call(JSEXT, JSEXT.$path);

  clib.chdir(args.cwd);

  // For out-of-tree code using the old names.  We can't just use a JSEXT1.js, because it would be ignored
  JSEXT1.__defineGetter__('dir', function() { return JSEXT1.os.dir; });
  JSEXT1.__defineGetter__('exists', function() { return JSEXT1.os.exists; });
  JSEXT1.__defineGetter__('getcwd', function() { return JSEXT1.os.getcwd; });
  JSEXT1.__defineGetter__('isdir', function() { return JSEXT1.os.isdir; });
  JSEXT1.__defineGetter__('stat', function() { return JSEXT1.os.stat; });


  // And now run as FastCGI, CGI, or an interactive shell

  if(environment.JSEXT_FCGI) {
    return JSEXT1.fcgi();
  }

  if(environment.GATEWAY_INTERFACE) {
    new JSEXT1.CGI();
    return;
  }

  return JSEXT1.interactive();
}

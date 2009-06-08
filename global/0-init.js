function(args) {
  this.Type = args.Type;
  this.Pointer = args.Pointer;
  this.Dl = args.Dl;
  this.load = args.load;
  this.environment = args.environment;
  this.gc = args.gc;
  this.isCompilableUnit = args.isCompilableUnit;
  this.jsxcore = args;

  load("0-ffi.js");

  $path=".";

  clib = load("clib.jswrapper");
  for(var i = 0; clib['dl ' + i]; i++) ;

  const path = args.environment.JSX_HOME + '/global/';
  this.JSEXT1 = {
    $path: path + 'JSEXT1',

    encodeUTF8: args.encodeUTF8,
    decodeUTF8: args.decodeUTF8,
    encodeJSON: args.encodeJSON,
    decodeJSON: args.decodeJSON,
    encodeBase64: args.encodeBase64,
    decodeBase64: args.decodeBase64,
  };

  const ActiveDirectory = JSEXT1.ActiveDirectory = load('JSEXT1/ActiveDirectory.js');
  JSEXT1.os = ActiveDirectory.handlers.js.call(JSEXT1, 'os', ".js"); // ActiveDirectory needs its .stat()
  ActiveDirectory.call(this, path);
  ActiveDirectory.call(JSEXT1, JSEXT1.$path);

  // For out-of-tree code using the old names.  We can't just use a JSEXT1.js, because it would be ignored
  JSEXT1.__defineGetter__('dir', function() { return JSEXT1.os.dir; });
  JSEXT1.__defineGetter__('exists', function() { return JSEXT1.os.exists; });
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

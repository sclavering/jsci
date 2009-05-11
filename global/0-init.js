function(Type, Pointer, Dl, load, cwd) {
  this.Type = Type;
  this.Pointer = Pointer;
  this.Dl = Dl;
  this.load = load;

  // avoid unwanted closures
  var xload = new Function("filename", "return load(filename);");

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

  // bootstrap enough of JSEXT1 that ActiveDirectory itself will work
  var mods = ['os', 'dir', 'isdir'];
  for(var i in mods) JSEXT[mods[i]] = ActiveDirectory.handlers.js.call(JSEXT, mods[i], ".js");

  JSEXT.$path = JSEXT.os.getcwd() + '/JSEXT1';
  JSEXT.ActiveDirectory = ActiveDirectory;

  ActiveDirectory.call(this, JSEXT.os.getcwd());
  ActiveDirectory.call(JSEXT, JSEXT.$path);

  JSEXT.chdir(cwd);

  // For out-of-tree code using the old names.  We can't just use a JSEXT1.js, because it would be ignored
  JSEXT1.__defineGetter__('getcwd', function() { return JSEXT1.os.getcwd; });
  JSEXT1.__defineGetter__('stat', function() { return JSEXT1.os.stat; });

  return JSEXT.shell;
}

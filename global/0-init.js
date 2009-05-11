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
  var mods = ['getcwd', 'os', 'dir', 'stat', 'isdir'];
  for(var i in mods) JSEXT[mods[i]] = ActiveDirectory.handlers.js.call(JSEXT, mods[i], ".js");

  JSEXT.$path = JSEXT.getcwd() + '/JSEXT1';
  JSEXT.ActiveDirectory = ActiveDirectory;

  ActiveDirectory.call(this, JSEXT.getcwd());
  ActiveDirectory.call(JSEXT, JSEXT.$path);

  JSEXT.chdir(cwd);

  return JSEXT.shell;
}

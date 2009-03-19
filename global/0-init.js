function(_dl, cwd, Type, Pointer, Dl, load) {
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

  const activate = xload('JSEXT1/activate.js');
  const js = activate.js;

  var mods = ['getcwd', 'os', 'dir', 'stat', 'isdir', 'ActiveDirectory'];
  for (var i in mods) JSEXT[mods[i]] = js.call(JSEXT, mods[i], ".js");

  JSEXT.$path = JSEXT.getcwd() + '/JSEXT1';
  JSEXT.activate = activate;

  JSEXT.ActiveDirectory.call(this, JSEXT.getcwd(), activate);
  JSEXT.ActiveDirectory.call(JSEXT, JSEXT.$path, activate);

  JSEXT.chdir(cwd);

  return JSEXT.shell;
}

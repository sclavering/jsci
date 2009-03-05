/*
    init(name, config, _dl, cwd, ...)

*/


function(name, config, _dl, cwd) {
  var mods = ['Type', 'Pointer', 'load', 'Dl'];
  for(var i in mods) this[mods[i]] = _dl('./' + mods[i] + '.so');

  var ActiveCdb = _dl('./JSEXT1/ActiveCdb.so');

  // avoid unwanted closures
  var xload = new Function("filename", "return load(filename);");

  $path=".";
  $curdir=this;

  clib = ActiveCdb("clib#__unix__.pch");

  var JSEXT = {
    ActiveCdb: ActiveCdb,
    $parent: this,
    $name: "JSEXT1",
    $path: './JSEXT1',
  };

  JSEXT.$curdir=JSEXT;

  for (var i=0; clib['dl '+i]; i++)
    ;

  this.JSEXT1 = JSEXT;

  const js = xload('JSEXT1/activate/js.js');

  var mods = ['getcwd', 'os', 'dir', 'stat', 'isdir', 'ActiveDirectory'];
  for (var i in mods) JSEXT[mods[i]] = js.call(JSEXT, mods[i], ".js");

  JSEXT.$path = JSEXT.getcwd() + '/JSEXT1';
  JSEXT.activate = new JSEXT.ActiveDirectory(JSEXT.getcwd() + '/JSEXT1/activate', { js: js }, config);

  JSEXT.ActiveDirectory.call(this, JSEXT.getcwd(), JSEXT.activate, config);

  $dirs.JSEXT=JSEXT;

  JSEXT.ActiveDirectory.call(JSEXT, JSEXT.$path, JSEXT.activate, config);

  JSEXT.chdir(cwd);

  return JSEXT.shell;
}

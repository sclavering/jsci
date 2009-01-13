/*
    init(name, config, _dl, cwd, ...)

*/


function(name, config, _dl, cwd) {
  config.pardir = '..';
  config.curdir =  '.';
  config.sep = '/';
  config.dlext = '.so';
  config.pathsep = ':';

  var JSEXT_version="1";

  var mods=['Type','Pointer','load','Dl'];
  for (var i in mods) {
    this[mods[i]]=_dl(config.curdir+config.sep+
		      mods[i]+config.dlext);
  }

  var ActiveCdb = _dl(config.curdir+config.sep+
		      "JSEXT"+JSEXT_version+config.sep+
		      "ActiveCdb"+config.dlext);

 // avoid unwanted closures
  var xload=new Function("filename",
			 "return load(filename);");

  $path=".";
  $curdir=this;
  JSEXT_config=config;

  clib = ActiveCdb("clib#__unix__.pch");

  var JSEXT = {
    ActiveCdb: ActiveCdb,
    $parent: this,
    $name: "JSEXT"+JSEXT_version,
    $path: config.curdir+config.sep+
           'JSEXT'+JSEXT_version,
  };

  JSEXT.$curdir=JSEXT;

  for (var i=0; clib['dl '+i]; i++)
    ;

  this['JSEXT'+JSEXT_version] = JSEXT;

  var js=xload('JSEXT'+JSEXT_version+config.sep+
	       'activate#host'+config.sep+
	       'js.js');

  var mods = ['getcwd', 'os', 'dir', 'stat', 'isdir', 'ActiveDirectory'];
  for (var i in mods) {
    JSEXT[mods[i]] = js.call(JSEXT,mods[i],".js");
  }

  JSEXT.$path = JSEXT.getcwd()+config.sep+
           'JSEXT'+JSEXT_version;
  JSEXT.activate=new JSEXT.ActiveDirectory(JSEXT.getcwd()+config.sep+
					   'JSEXT'+JSEXT_version+config.sep+
					   'activate#host',
					   {js: js},
					   config);

  JSEXT.ActiveDirectory.call(this,
			     JSEXT.getcwd(),
			     JSEXT.activate,
			     config);

  $dirs.JSEXT=JSEXT;

  JSEXT.ActiveDirectory.call(JSEXT,
			     JSEXT.$path,
			     JSEXT.activate,
			     config);

  JSEXT.chdir(cwd);

  return JSEXT.shell;
}

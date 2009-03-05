/*
  This handler handles .h, .c, .pch, .so and .dll files.

  .h and .c files are source files. .pch, .so and .dll files
  are generated automatically from .h and .c files if necessary.

  If a .c file exists, it is compiled as a dynamic library
  and used as the default dl file when parsing the .h file.

  If no .h file exists (but a .c file), then function declarations
  are extracted from the .c file.
  
*/

function(name, extension) {
  // 1. Find timestamps of all files in group

  var timestamp={};

  //clib.puts("cget "+this.$path+"/"+name);
  for each(var ext in ['h', 'c', 'pch', 'so']) {
    var stat = JSEXT1.stat(this.$path + '/' + name + '.' + ext);
    timestamp[ext]=stat && stat.mtime;
  }

  // 2. Build dl file if possible and necessary

  var dlobj;

  if(timestamp.c && timestamp.c > timestamp.so) {
    //clib.puts("ccget "+name);
    JSEXT1.chdirLock(this.$path);
    JSEXT1.C.compile(name+'.c');
    var stat = JSEXT1.stat(name + '.so');
    timestamp.so = stat && stat.mtime;
    JSEXT1.chdirUnlock();
  }
  //clib.puts("dget "+name);

  if(timestamp.so) {
    //clib.puts("eget "+name+" "+this.$path);
    JSEXT1.chdirLock(this.$path);
    //clib.puts("dlget "+name);
    dlobj = Dl('./' + name + '.so');
    //clib.puts("dlgot "+name);
    JSEXT1.chdirUnlock();
  }
  //clib.puts("fget "+name);

  // 3. If it is a jsapi module, just load it.

  if (dlobj && dlobj.symbolExists('JSX_init')) {
    //clib.puts("dlini "+name);
    var ret=dlobj['function']('JSX_init')();
    //clib.puts("dlino "+name);
    return ret;
  }

  // 4. Build pch file if possible and necessary

  if (timestamp.h && timestamp.h > timestamp.pch) {
    JSEXT1.chdirLock(this.$path);
    var ps=new JSEXT1.Progress;
    ps.status("Parsing h file");
    var fragment=JSEXT1.C.fragment(JSEXT1.read(name+'.h'), dlobj);
		ps.status("Writing cdb file");
    var pch=new JSEXT1.Cdb(name+'.pch','w');
    pch.write(fragment);
    pch.close();
    ps.close();
    timestamp.pch=JSEXT1.stat(name+'.pch');
    JSEXT1.chdirUnlock();
  }

  // 5. Load pch file if possible

  if (timestamp.pch) {
    JSEXT1.chdirLock(this.$path);
    var ret=new JSEXT1.ActiveCdb(name+".pch");
    var i;
    for (i=0; ret['dl '+i]; i++)
      ;
    JSEXT1.chdirUnlock();
    if (ret.hasOwnProperty("main"))
      return ret.main;
    return ret;
  }

}

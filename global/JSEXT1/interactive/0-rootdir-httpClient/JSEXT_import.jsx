function(dep, predef) {
  var pathparts=this.filename.split(JSEXT_config.sep);
  var curdir=this.clientdir;

  for (var i=1; i<pathparts.length-1; i++) {
    curdir=curdir[pathparts[i]];
  }

  return JSEXT.js['export'](dep, predef, curdir);
}

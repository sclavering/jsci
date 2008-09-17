function(cmd) {
  if (!JSEXT1.js.isCompilableUnit(cmd))
    return false;
  var global=function(){return this;}();
  global.$checkdates();
  JSEXT1.interactive.execline(cmd);
}


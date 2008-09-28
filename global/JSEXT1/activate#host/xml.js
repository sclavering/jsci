/*
  Loads a file and calls [[$parent.soap.toXML]].

 */

function(name,extension) {
  var file=new JSEXT1.File(this.$path+JSEXT_config.sep+name+extension,"r");
  var ret=file.read();
  file.close();
  return JSEXT1.soap.toXML(ret);
}

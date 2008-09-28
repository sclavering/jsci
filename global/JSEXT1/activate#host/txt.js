/*
  Loads the file and returns contents as an 8-bit string.

 */

function(name,extension) {
  var file=new JSEXT1.File(this.$path+JSEXT_config.sep+name+extension,"r");
  var ret=file.read();
  file.close();
  return ret;
}

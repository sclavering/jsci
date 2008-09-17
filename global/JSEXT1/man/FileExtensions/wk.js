function (name,extension) {
  var file=new $parent.$parent.File(this.$path+JSEXT_config.sep+name+extension,"r");
  var text = file.read();
  file.close();
  
  return {$doc:{text:text}};
}

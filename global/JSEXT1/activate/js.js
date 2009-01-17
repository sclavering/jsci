/*

  JavaScript files are interpreted with the current directory's
  ActiveDirectory object on top of the scope chain.

 */

function (name,extension) {
  var w;
  if (name!="with" && (w=this['with'])) {
    var before="";
    for (var i=w.length; i--;)
      before+="with(this['with']["+i+"])";
    before+="with(this){";
    return load.call(this, this.$path+JSEXT_config.sep+name+extension,before,"}");
  } else {
    return load.call(this, this.$path+JSEXT_config.sep+name+extension);
  }
}

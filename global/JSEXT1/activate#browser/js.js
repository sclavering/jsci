function (name,extension) {
  try {
    var dep=JSEXT1.http.get(this.$url+"/"+encodeURIComponent(name)+'.dep').document;
  } catch(x) {}
  if (dep) {
    dep=eval(dep);
    var i;
    for (i=0; i<dep.length; i++)
      JSEXT1.require(dep[i],this);
  }
  var script=JSEXT1.http.get(this.$url+"/"+encodeURIComponent(name)+extension).document;
  with(this) {
    try { return eval.call(this, '('+script+')||undefined'); } catch(x) {}
    try { return eval.call(this, script); } catch(x) {}
  }
}

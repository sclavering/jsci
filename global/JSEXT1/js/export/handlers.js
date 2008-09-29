({
  jsx: function(filename, extension) {
    var path=[];
    var curdir=this;
    while (!curdir.$url) {
      path.unshift(curdir.$name+"/")
	curdir=curdir.$parent;
    }
    return {
      $_url: curdir.$url+path.join("")+filename+extension
    };
  }
})

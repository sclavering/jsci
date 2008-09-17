/*
     Handles .js and .xdk files
    */

function (name, extension) {
    // 1. Find timestamps of all files in group
    
    var timestamp={};
    
    for each (var ext in ['js', 'xdk']) {
      var stat=$parent.$parent.stat(this.$path+JSEXT_config.sep+name+'.'+ext);
      timestamp[ext]=stat && stat.mtime;
    }
    
    if (timestamp.js && timestamp.js > timestamp.xdk) {

      // The $parent object is a documentation object. To
      // properly load the .js file, it is necessary to
      // find the $parent object of the .js file.

      // Find path from here to global object
      
      var cxpath=[];
      var cur=this;
      
      while (cur.$name) {
	cxpath.unshift(cur.$name);
	cur=cur.$parent;
      }
      
      // Follow path to find context for js file
      
      var cur=(function(){return this;})(); //global
      while (cxpath.length) {
	cur=cur[cxpath.shift()];
      }
      
      var xdk=$parent.extract.call(cur,this.$path+JSEXT_config.sep+name+".js");
      $parent.$parent.write(this.$path+JSEXT_config.sep+name+".xdk",xdk.toSource());
      return xdk;
    }

    return load(this.$path+JSEXT_config.sep+name+".xdk");
}

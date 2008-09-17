  /*
          filename(filename)
          
      Returns filename part of the filename and path
     */

(function() {

  var regexp=new RegExp("(.*)"+JSEXT_config.sep+"([^"+JSEXT_config.sep+"]*)$");

  return function(filename) {
    var m=filename.match(regexp);
    if (m) {
      return m[2];
    }
    return filename;
  }

})()

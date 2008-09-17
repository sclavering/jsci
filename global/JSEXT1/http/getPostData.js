/*

      obj = http.getPostData.call(this)

  Returns an object with the name/value pairs given by the posted data
  in [[stdin]] and 'this.requestHeaders'.
  
 */

function() {
  var post;

  if (this.method=="POST") {
    var ct=$parent.mime.nameValuePairDecode(this.requestHeaders.contentType);
    
    if (ct=="text/JSON") {
      var str=stdin.read();
      post=$parent.decodeJSON(str);
    } else if (ct=="application/x-www-form-urlencoded") {
      var str=stdin.read();
      post=decodeQry(str);
    } else if (ct=="multipart/form-data") {
      var stream=new $parent.StringFile(stdin.read()); // Sorry, must read into ram because readln is not bin-safe.

      post=$parent.mime.decodeMultipart(stream, ct.boundary);
    }
  }

  return post || {};
}

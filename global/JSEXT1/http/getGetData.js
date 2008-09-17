/*

      obj = http.getGetData.call(this)

  Returns an object with the name/value pairs given by the query string
  in 'this.requestURL'.
  
 */

function() {
    var parts=decodeURI(this.requestURL);
    return parts.qry || {};
}
/*

      obj = http.getFormData.call(this)

  Returns an object with the name/value pairs given by the form data
  in 'this.requestURL' and (if using the POST method) [[stdin]] and 'this.requestHeaders'.
  _Post_ elements take precedence over _get_ elements by the same name.
  
 */


function () {
  var get=getGetData.call(this);
  var post=getPostData.call(this);

  var hasOwnProperty=Object.prototype.hasOwnProperty;
  for (var i in post)
    if (hasOwnProperty.call(post,i))
      get[i]=post[i];

  return get;
}

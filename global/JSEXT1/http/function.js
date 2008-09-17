/*
         func = http['function'](url) 

     Create a function which calls a function on another host,
     using the HTTP protocol to transfer parameters and return value.
     Since _function_ is a reserved word, you need to use brackets
     and quotes like above. That is, _http.function_ is a syntax error,
     while _http['function']_ works.

     _http.function_ is normally called for you by the code generated
     by [[$curdir.exportScript]].

     ### Arguments ###

     * _url_: A string containing the url to the function,

     ### Return value ###

     This function returns a function. Let us call it _func_.
     When _func_ is called, it will send a regular HTTP POST request to the
     specified url, using multipart/form-data encoding. Argument no. 0
     is named "0", argument no. 1 is named "1" etc.

     The response
     body from the server, as a string, is the return value from _func_.

     If any of the arguments to _func_ is not a string, then it
     is encoded as JSON, and the content-type of the corresponding
     form element is set to "text/JSON".

     If the content-type of the returned document is "text/JSON",
     the content body is decoded as JSON before it is returned.

     Note that this corresponds to the way that arguments and
     return values are
     handled when JSEXT executes _.jsx_ files: Any arguments
     passed to _func_ will show up as the same argument number
     in your _.jsx_ file on the server, after going through
     JSON encoding and decoding if necessary.
     Your function's return value is
     JSON encoded for you if necessary and decoded by _func_ before it
     is returned to the caller of _func_.

*/

function(url) {
  function wrapper() {
    var a=[];
    for (var i=0; i<arguments.length; i++)
      a[i]=arguments[i];

    var r=post(url, a);
    if ('stream' in r) {
      if (r.headers.contentType=="text/JSON")
	return $parent.decodeJSON(r.stream.read());
      else
        return r.stream.read();
    } else
      throw new Error("Unable to call '"+url+"': "+r.statusLine);
  }

  wrapper.async=function(callback, Catch) {
    var a=[];
    for (var i=2; i<arguments.length; i++)
      a[i-2]=arguments[i];
    post(url, a, {}, {callback: funcCallback, 'catch': Catch});

    function funcCallback(r) {

      if ('stream' in r) {
	if (r.headers.contentType=="text/JSON")
	  callback($parent.decodeJSON(r.stream.read()));
	else
	  callback(r.stream.read());
      } else {
	var err=new Error("Unable to call '"+url+"': "+r.statusLine);
	if (Catch)
	  Catch(err);
	else
	  throw err;
      }
    }

  }

  return wrapper;


}

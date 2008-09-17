/*
    str = read(uri)

Opens _uri_ using [[$curdir.open]], reads entire contents and closes.
Returns a string.
*/

function(uri) {
  var f=open(uri);
  var ret=f.read();
  f.close();
  return ret;
}

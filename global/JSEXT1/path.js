/*
path(filename)

Returns path part of the filename and path
*/
function(filename) {
  var m = filename.match("(.*)/([^/]*)$");
  if(m) return m[1] != "" ? m[1] : '/';
  return '.';
}

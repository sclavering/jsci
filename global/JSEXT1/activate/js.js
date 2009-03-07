/*
JavaScript files are interpreted with the current directory's ActiveDirectory object on top of the scope chain.
*/
function(name, extension) {
  return load.call(this, this.$path + '/' + name + extension);
}

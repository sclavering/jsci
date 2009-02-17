/*
open(filename, mode)

An alias for:  new File(filenmae mode)

(Formerly dealt with URIs too.)
*/
function(filename, mode) {
  return new JSEXT1.File(uri.fullPath, mode);
}

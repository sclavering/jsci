/*

    stream = open("file:///path/file" [, mode])

  Opens a local file. The second argument to _open_ is passed as
  the _mode_ ("r", "w", etc) to [[$parent.File]].

 */

function(uri, mode) {
  return new $parent.File(uri.fullPath, mode);
}

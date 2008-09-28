/*
  Opens an SQLite database and returns the
  corresponding [[$parent.Sqlite]] object.
new JSX10.File() */

function(name,extension) {
  return new JSEXT1.Sqlite(this.$path+JSEXT_config.sep+name+extension);
}

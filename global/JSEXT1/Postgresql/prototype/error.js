function() {
  return lib.PQerrorMessage(this.conn).string().replace(/\n$/,"");
}

function() {
  return lib.PQresultErrorMessage(this.res).string().replace(/\n$/,"");
}

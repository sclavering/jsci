function() {
  if (!this.closed) {
    this.closed=true;
    lib.PQfinish(this.conn);
    this.conn.finalize=null;
  }
}

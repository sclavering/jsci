/**/

function() {
  if (this.result)
    this.free();
  
  lib.mysql_close(this.mysql);
  this.mysql.finalize=null;
  delete this.mysql;
}

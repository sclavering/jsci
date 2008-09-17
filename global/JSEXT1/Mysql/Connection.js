/*
  Represents a connection to a MySQL server. These connections
  are created automatically by the [[$parent]] object.

 */

function(parent, params) {
  this.parent=parent;

  this.mysql=lib.mysql_init(null);
  this.mysql.finalize=lib.mysql_close;
  lib.mysql_options(this.mysql, lib.MYSQL_OPT_RECONNECT, Type.pointer(Type.int), [1]); // automatic reconnect
  if (!lib.mysql_real_connect(this.mysql,
				    params.host,
				    params.user,
				    params.passwd,
				    params.db,
				    params.port,
				    params.unix_socket,
				    params.clientflag)) {
    this.throwError();
  }
  this.exec("SET NAMES 'utf8'");
}

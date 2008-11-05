/*
new MysqlConnection(params)

An object representing a single connection to a MySQL database.

### Arguments ###

 * _params_: An object which may contain these properties:
   * _host_: The host where the server runs
   * _user_: The mysql username
   * _passwd_
   * _db_: Which database to use
   * _port_
   * _unix\_socket_
   * _clientflag_
*/

function(params) {
  params = params || {};

  if (params.host===undefined) params.host=null;
  if (params.user===undefined) params.user=null;
  if (params.passwd===undefined) params.passwd=null;
  if (params.db===undefined) params.db=null;
  if (params.port===undefined) params.port=null;
  if (params.unix_socket===undefined) params.unix_socket=null;
  if (params.clientflag===undefined) params.clientflag=0;
  
  this.params = params;

  const lib = JSEXT1.libmysql;
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
  this._exec(["SET NAMES 'utf8'"]);
}

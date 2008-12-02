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
  this.params = params = params || {};

  if (params.host===undefined) params.host=null;
  if (params.user===undefined) params.user=null;
  if (params.passwd===undefined) params.passwd=null;
  if (params.db===undefined) params.db=null;
  if (params.port===undefined) params.port=null;
  if (params.unix_socket===undefined) params.unix_socket=null;
  if (params.clientflag===undefined) params.clientflag=0;
  
  this._mysql = libmysql.mysql_init(null);
  this._mysql.finalize = libmysql.mysql_close;
  libmysql.mysql_options(this._mysql, libmysql.MYSQL_OPT_RECONNECT, Type.pointer(Type.int), [1]); // automatic reconnect
  if (!libmysql.mysql_real_connect(this._mysql,
				    params.host,
				    params.user,
				    params.passwd,
				    params.db,
				    params.port,
				    params.unix_socket,
				    params.clientflag)) {
    this._throw_error();
  }
  this._exec(["SET NAMES utf8"]);
}

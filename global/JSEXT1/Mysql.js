/*
               new Mysql(params)

           An object representing a MySQL database. A connection is
           made on the first query. New connections
           are made as necessary, since each connection
           can only handle one query at a time.
	   This can happen in a threaded environment, or when
           using unbuffered queries (using the _prepare_ method). A pool
           of connections is maintained and reused. No connections are
           closed before the MySQL object is garbage collected, or the
           [[$curdir.Mysql.prototype.close]] method is called.

           ### Arguments ###

           * _params_: An object which may contain these properties:

             * _host_: The host where the server runs
             * _user_
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
  this.freeConnections = [];
  this.connections = [];

}

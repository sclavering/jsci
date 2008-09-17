/*
               new Postgresql(params)

           An object representing a Postgresql connection.

           ### Arguments ###

           * _params_: An object which may contain these properties:

             * _host_: Name of host to connect to
	     * _hostaddr_: Numeric IP address of host to connect to. Without either a host name or host address, libpq will connect using a local Unix-domain socket; or on machines without Unix-domain sockets, it will attempt to connect to localhost.
	     * _port_: Port number to connect to at the server host, or socket file name extension for Unix-domain connections.
             * _dbname_: Which database to use
             * _user_
             * _password_
	     * _connect\_timeout_: In seconds. Default is 0 (indefinite)
	     * _options_: Command-line options to be sent to the server
	     * _sslmode_: This option determines whether or with what priority an SSL connection will be negotiated with the server. There are four modes: disable will attempt only an unencrypted SSL connection; allow will negotiate, trying first a non-SSL connection, then if that fails, trying an SSL connection; prefer (the default) will negotiate, trying first an SSL connection, then if that fails, trying a regular non-SSL connection; require will try only an SSL connection. 
	     * _requiressl_: 1 or 0 (not boolean)
	     * _krbsrvname_: Kerberos service name
	     * _gsslib_: GSS library to use for GSSAPI authentication.
	     * _service_: Service name to use for additional parameters
	     
	     
    */

function(params) {

  params = params || {};

  var pstr="";
  for (var i in params)
    if (params.hasOwnProperty(i)) {
      pstr+=" "+i+"='"+params[i].replace(/([\\\'])/g,"\\$1")+"'";
  }

  var lib=arguments.callee.lib;

  this.conn=lib.PQconnectdb(pstr.substr(1));
  if (!this.conn)
    throw new Error("Postgresql");
  this.conn.finalize=lib.PQfinish;
  if (lib.PQstatus(this.conn) != lib.CONNECTION_OK) {
    var err=this.error();
    this.close();
    throw Error(err);
  }
  lib.PQsetClientEncoding(this.conn, "UTF8");
}

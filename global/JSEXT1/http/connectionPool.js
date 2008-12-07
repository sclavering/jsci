/*

         connectionPool(host, port, header, body, options)

     Sends _header_ and _body_ to
     _host:port_, using a pooled connection if possible.

     Returns response from [[$curdir.readMessage]].
*/

(function(curdir){

  var pool={};
  var queue=[];
  var maxconn=4;

  return function(host, port, header, body, options) {

    if (pool[host+':'+port]) {
      try {

	var conn=pool[host+':'+port];

	conn.write(header);

	if (body) {
	  if (typeof body==="function")
	    body(conn);
	  else
	    conn.write(body);
	  conn.write("\r\n"); // footers
	}

	conn.flush();

	return readMessage(conn);
	
      } catch(x) {
	conn.close();
	for (var i=0; i<queue.length; i++) {
	  if (queue[i]==host+':'+port) {
	    queue.splice(i, 1);
	  }
	}
	delete pool[host+':'+port];
      }
    }

    if (queue.length==maxconn) {
      var oldconn=queue.shift();
      pool[oldconn].close();
      delete pool[oldconn];
    }

    var conn=$parent.tcp.connect(host, port);

    pool[host+':'+port]=conn;
    queue.push(host+':'+port);

    conn.write(header);

    switch(typeof body) {
    case 'function':
      var chunkStream=new ChunkWriter(conn);
      body(chunkStream);
      chunkStream.close();
      conn.write("\r\n"); // footers
      break;
    case 'undefined':
      break;
    default:
      conn.write(body);
    }


    conn.flush();

    return readMessage(conn);

  }

})(this)


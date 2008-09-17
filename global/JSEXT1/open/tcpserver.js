/*

    stream = open("tcpserver://host:port/" [, options])

Opens a tcp port for listening. If you wish to listen on the
same port on all interfaces, give an empty host name like this:

    tcpserver://:5000/

An optional second argument passed to [[$curdir]] is passed
on to [[$parent.tcp.Listen]]'s _options_ argument.

 */

function(uri, options) {
    return new $parent.tcp.Listen(uri.host, uri.port, options);
}


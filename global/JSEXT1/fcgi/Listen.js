(function(){

    /*

         new Listen([path], [backlog])

     Listens to an fcgi socket.

     */

  function f(path, backlog) {
    if (backlog===undefined)
      backlog=8;

    if (path===undefined) {
      this.fd=0;
    } else {
      this.fd=lib.FCGX_OpenSocket(path,backlog);
      if (this.fd==-1) throw new Error("fcgi:OpenSocket");
    }
    
    
  }

  f.prototype={
  /*
        req = obj.accept()

    Returns a new [[$parent.$parent.Request]] object
    as soon as a new connection is available.

   */

    accept: function() {
      return new Request(this.fd);
    },

  /*
        obj.close()

    Closes the fcgi socket.

   */

    close: function() {
      if (this.fd)
        clib.close(this.fd);
    }
  }

  return f;

})()

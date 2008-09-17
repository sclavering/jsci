/*

    obj = new Request(fd)

Object which represents a request from an HTTP server to an
fcgi server.

Contains the following properties:

* _in_: A [[$curdir.File]] object representing the per-request standard input
* _out_: A [[$curdir.File]] object representing the per-request standard output
* _err_: A [[$curdir.File]] object representing the per-request standard error
* _env_: An object containing the per-request environment variables

### Arguments ###

* _fd_: The file descriptor used for communication with the HTTP server.

*/

(function() {

  function f(fd) {
    this.request=new Pointer(lib['struct FCGX_Request']);
    var res=lib.FCGX_InitRequest(this.request, fd, 0);

    if (res!=0)
      throw new Error("FCGX_InitRequest");
    
    var res=lib.FCGX_Accept_r(this.request);
    if (res!=0)
      throw new Error("FCGX_Accept_r");
    this.request.finalize=lib.FCGX_Finish_r;
    
    this['in']=new File(this.request.member(0,"in").$);
    this['out']=new File(this.request.member(0,"out").$);
    this['err']=new File(this.request.member(0,"err").$);
    this['env']={};
    
    var envp=this.request.member(0,"envp").$;
    if (envp==null) return;
    var i;
    for(i=0;;i++) {
      var val=envp.member(i).$;
      if (val==null) break;
      val=val.string();
      var eqPos=val.indexOf("=");
      this.env[val.substr(0,eqPos)]=val.substr(eqPos+1);
    }
  }

/*


    req.close()

Finish & free the request (multi-thread safe).

Side effect:
---

Finishes the request accepted by (and frees any
storage allocated by) the previous call to accept.


*/

  f.prototype.close=function() {
    this.request.finalize=null;
    lib.FCGX_Finish_r(this.request);
  }

  return f;

})()

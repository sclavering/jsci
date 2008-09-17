/*

     str = infl.read( [n] )

  Decompresses the underlying stream to return a string which is _n_ bytes long, or
  until the end of the compressed data. If _n_ is omitted, reads until the end of
  the compressed data.

 */

function(n) {
  var self=this;
  var inbuf=new Pointer(Type.array(Type.char,65536));
  var outbuf=new Pointer(Type.array(Type.char,65536));

  if (n) {
    while (this.buf.length < n) {
      if (readblock())
	break;
    }
    var ret=this.buf.substr(0,n);
    this.buf=this.buf.substr(n);
    return ret;
  }

  while (!readblock());
  var ret=this.buf;
  this.buf="";
  return ret;
  
  function readblock() {
    if (self.source.eof()) {
      self.iseof=true;
      return true;
    }
    var inblock=self.source.read(65536);
    clib.memcpy(inbuf, Type.pointer(Type.char), inblock, inblock.length);
    self.z_stream.member(0, "avail_in").$=inblock.length;
    self.z_stream.member(0, "next_in").$=inbuf;
    var flush=self.source.eof();
    do {
      self.z_stream.member(0, "avail_out").$=65536;
      self.z_stream.member(0, "next_out").$=outbuf;
      var ret=lib.inflate(self.z_stream, flush);
      if (ret != lib.Z_OK && ret != lib.Z_STREAM_END)
	throw new Error ("zlib");
      var avail_out=self.z_stream.member(0,"avail_out").$;
      self.buf+=outbuf.string(65536-avail_out);
    } while (avail_out==0);
    return ret == lib.Z_STREAM_END;
  }
}

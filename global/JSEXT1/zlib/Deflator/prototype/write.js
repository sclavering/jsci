/*
      defl.write(str)

  Deflates a string and writes the compressed data to the underlying stream.

 */

function(str) {
  str=String(str);

  var inbuf=new Pointer(Type.array(Type.char,65536));
  var ret="";

  while (str.length) {
    var avail_in=Math.min(str.length, 65536);
    clib.memcpy(inbuf, Type.pointer(Type.char), str.substr(0, avail_in), avail_in);
    this.z_stream.member(0, "next_in").$=inbuf;
    this.z_stream.member(0, "avail_in").$=avail_in;
    this.flush(lib.Z_NO_FLUSH);
    str=str.substr(65536);
  }
}

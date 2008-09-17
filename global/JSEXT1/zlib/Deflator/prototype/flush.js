/*

      defl.flush()

  Writes as much as possible of the data to the underlying stream. Since
  zlib does not write one byte at a time, there is no guarantee that all
  data is present in the underlying stream until the deflator is closed.

 */


function(flag) {
  if (flag===undefined)
    flag=lib.Z_SYNC_FLUSH;

  var outbuf=new Pointer(Type.array(Type.char,65536));

  do {
    this.z_stream.member(0, "next_out").$=outbuf;
    this.z_stream.member(0, "avail_out").$=65536;
    
    var ret=lib.deflate(this.z_stream, flag);
    if (ret != lib.Z_OK && ret != lib.Z_STREAM_END)
      throw new Error("zlib");
    var avail_out=this.z_stream.member(0,"avail_out").$;
    this.dest.write(outbuf.string(65536-avail_out));
  } while (avail_out==0);
  
}

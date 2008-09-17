/*
  (version >= 1.1)
  
      infl = new Inflator(stream [, gzip])


  A file-like object which inflates (decompresses) the data
  in _stream_ when you read from it.

  ### Arguments ###

  * _stream_: The file-like object to read compressed data from.
  * _gzip_: Boolean. The stream contains a gzip header. Default=true

 */



function(source, gzip) {
  if (gzip===undefined)
    gzip=true;

  this.source=source;
  this.z_stream=new Pointer(lib.z_stream);
  this.z_stream.member(0,"zalloc").$=lib.Z_NULL;
  this.z_stream.member(0,"zfree").$=lib.Z_NULL;
  this.z_stream.member(0,"opaque").$=lib.Z_NULL;
  this.z_stream.member(0,"avail_in").$=0;
  this.z_stream.member(0,"next_in").$=0;
  if (lib.inflateInit2_(this.z_stream,
			15 + (gzip?16:0),			
			lib.ZLIB_VERSION, lib.z_stream.sizeof) != lib.Z_OK)
    throw new Error ("zlib");
  this.buf="";
}

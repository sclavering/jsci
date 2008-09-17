/*
  (version >= 1.1)

      defl = new Deflator(stream [, level [, gzip]])

  A file-like object which deflates (compresses) the data
  written to it. The compressed data is written to the file-like object
  _stream_.

  ### Arguments ###

  * _stream_: The file-like object to write compressed data to.
  * _level_: Number. The compression level (1-9) Default=6
  * _gzip_: Boolean. Write a gzip header to produce a valid .gz file. Default=true

 */

function(dest, level, gzip) {
  if (level===undefined)
    level=lib.Z_DEFAULT_COMPRESSION;
  if (gzip===undefined)
    gzip=true;

  this.dest=dest;
  this.z_stream=new Pointer(lib.z_stream);
  this.z_stream.member(0,"zalloc").$=lib.Z_NULL;
  this.z_stream.member(0,"zfree").$=lib.Z_NULL;
  this.z_stream.member(0,"opaque").$=lib.Z_NULL;
  //  if (lib.deflateInit_(this.z_stream, level, lib.ZLIB_VERSION, lib.z_stream.sizeof) != lib.Z_OK)
  //    throw new Error ("zlib");
  if (lib.deflateInit2_(this.z_stream,
			level,
			lib.Z_DEFLATED,
			15 + (gzip?16:0),
			8,
			lib.Z_DEFAULT_STRATEGY,
			lib.ZLIB_VERSION, lib.z_stream.sizeof) != lib.Z_OK)
    throw new Error ("zlib");
  this.buf="";
}

/*
      defl.close()

  Closes the deflator. Does not close the underlying stream.

 */

function() {
  this.z_stream.member(0, "avail_in").$=0;
  this.flush(lib.Z_FINISH);

  lib.deflateEnd(this.z_stream);
}

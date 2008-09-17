/*
      infl.close()

  Closes the inflator, but does not touch the underlying stream.
 */

function() {
  lib.inflateEnd(this.z_stream);
}
  /*
    times()

Return an object with integer numbers indicating process times.
     */

function() {
  var t=Pointer(clib['struct tms']);
  var res=clib.times(t);
  if (res==-1) {
    throw new Error(os.error("times"));
  }
  
  return t.$;
}

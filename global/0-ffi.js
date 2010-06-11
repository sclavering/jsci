/*
Implement bits of Pointer, Type, and Dl that don't need to be done in C
*/

this.Type = jsxlib.Type;
this.Pointer = jsxlib.Pointer;
this.Dl = jsxlib.Dl;
this.load = jsxlib.load;
this.environment = jsxlib.environment;
this.gc = jsxlib.gc;
this.isCompilableUnit = jsxlib.isCompilableUnit;

// xxx not portable (e.g. char can be unsigned)
Type.char = Type.signed_char;
Type.short = Type.signed_short;
Type.int = Type.signed_int;
Type.long = Type.signed_long;
Type.long_long = Type.signed_long_long;

// Not sure if these are really necessary
Type.int64 = Type.long_long;
Type.signed_int64 = Type.signed_long_long;
Type.unsigned_int64 = Type.unsigned_long_long;

Pointer.prototype.toString = function() {
  return Number(this.valueOf()).toString(16);
};

Pointer.prototype.__defineSetter__("finalizer", Pointer.prototype.setFinalizer);

/*
Implement bits of Pointer, Type, and Dl that don't need to be done in C
*/

// xxx not portable (e.g. char can be unsigned)
Type.char = Type.signed_char;
Type.short = Type.signed_short;
Type.int = Type.signed_int;
Type.long = Type.signed_long;
Type.long_long = Type.signed_long_long;

Type.unsigned_char.toString = function() { return "unsigned char"; };
Type.signed_char.toString = function() { return "signed char"; };
Type.unsigned_short.toString = function() { return "unsigned short"; };
Type.signed_short.toString = function() { return "signed short"; };
Type.unsigned_int.toString = function() { return "unsigned int"; };
Type.signed_int.toString = function() { return "signed int"; };
Type.unsigned_long.toString = function() { return "unsigned long"; };
Type.signed_long.toString = function() { return "signed long"; };
Type.unsigned_long_long.toString = function() { return "unsigned long long"; };
Type.signed_long_long.toString = function() { return "signed long long"; };

Type.float.toString = function() { return "float"; };
Type.double.toString = function() { return "double"; };
Type.long_double.toString = function() { return "long double"; };

Type['void'].toString = function() { return "void"; };

Type.array_prototype.toString = function() { return "array"; };
Type.bitfield_prototype.toString = function() { return "bitfield"; };
Type.function_prototype.toString = function() { return "function"; };
Type.pointer_prototype.toString = function() { return "pointer"; };
Type.struct_prototype.toString = function() { return "struct"; };
Type.union_prototype.toString = function() { return "union"; };

// Not sure if these are really necessary
Type.int64 = Type.long_long;
Type.signed_int64 = Type.signed_long_long;
Type.unsigned_int64 = Type.unsigned_long_long;

Pointer.prototype.toString = function() {
  return Number(this.valueOf()).toString(16);
};

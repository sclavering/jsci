/*
Implement bits of Pointer, Type, and Dl that don't need to be done in C
*/

Type.unsigned_char.toString = function() { return "unsigned char"; };
Type.signed_char.toString = function() { return "signed char"; };
Type.char.toString = function() { return "char"; };
Type.unsigned_short.toString = function() { return "unsigned short"; };
Type.signed_short.toString = function() { return "signed short"; };
Type.short.toString = function() { return "short"; };
Type.unsigned_int.toString = function() { return "unsigned int"; };
Type.signed_int.toString = function() { return "signed int"; };
Type.int.toString = function() { return "int"; };
Type.unsigned_long.toString = function() { return "unsigned long"; };
Type.signed_long.toString = function() { return "signed long"; };
Type.long.toString = function() { return "long"; };
Type.unsigned_long_long.toString = function() { return "unsigned long long"; };
Type.signed_long_long.toString = function() { return "signed long long"; };
Type.long_long.toString = function() { return "long long"; };
Type.unsigned_int64.toString = function() { return "unsigned int64"; };
Type.signed_int64.toString = function() { return "signed int64"; };
Type.int64.toString = function() { return "int64"; };
Type.float.toString = function() { return "float"; };
Type.double.toString = function() { return "double"; };
Type.long_double.toString = function() { return "long double"; };
Type['void'].toString = function() { return "void"; };

Pointer.prototype.toString = function() {
  return Number(this.valueOf()).toString(16);
};

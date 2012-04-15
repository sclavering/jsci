#include "jsci.h"

#include <stdlib.h>
#include <string.h>



JsciType::~JsciType() {
}


JSBool JsciType::JStoC(JSContext *cx, char *data, jsval v) {
  return JSX_ReportException(cx, "Bad JStoC call");
}


ffi_type *JsciType::GetFFIType() {
  return 0;
}


int JsciType::SizeInBits() {
  return this->SizeInBytes() * 8;
}


int JsciType::SizeInBytes() {
  return 0; // a nonsensical value
}


int JsciType::AlignmentInBits() {
  return this->AlignmentInBytes() * 8;
}


int JsciType::AlignmentInBytes() {
  return 0; // nonsensical default
}





JsciTypeVoid::JsciTypeVoid() {
}


int JsciTypeVoid::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JSX_ReportException(cx, "Cannot convert C value of type void to a JS value");
}


ffi_type *JsciTypeVoid::GetFFIType() {
  return &ffi_type_void;
}





JsciTypeNumeric::JsciTypeNumeric(ffi_type ffit) : ffiType(ffit) {
}


ffi_type *JsciTypeNumeric::GetFFIType() {
  return &this->ffiType;
}


int JsciTypeNumeric::SizeInBytes() {
  return this->ffiType.size;
}


int JsciTypeNumeric::AlignmentInBytes() {
  return this->ffiType.alignment;
}





JsciTypeFloat::JsciTypeFloat() : JsciTypeNumeric(ffi_type_float) {
}


JSBool JsciTypeFloat::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JS_NewNumberValue(cx, *(float*) data, rval);
}


JSBool JsciTypeFloat::JStoC(JSContext *cx, char *data, jsval v) {
  jsdouble tmpdouble;
  if(!JS_ValueToNumber(cx, v, &tmpdouble)) return JS_FALSE;
  *(float*)data = tmpdouble;
  return JS_TRUE;
}



JsciTypeDouble::JsciTypeDouble() : JsciTypeNumeric(ffi_type_double) {
}


JSBool JsciTypeDouble::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JS_NewNumberValue(cx, *(double*) data, rval);
}


JSBool JsciTypeDouble::JStoC(JSContext *cx, char *data, jsval v) {
  return JS_ValueToNumber(cx, v, (double*) data);
}





JsciTypeInt::JsciTypeInt(int size, ffi_type ffit) : JsciTypeNumeric(ffit), size(size) {
}


JSBool JsciTypeInt::CtoJS(JSContext *cx, char *data, jsval *rval) {
  int tmp;
  // Return a number from an unsigned int (of various sizes)
  switch(this->size) {
    case 0: tmp = *(char *) data; break;
    case 1: tmp = *(short *) data; break;
    case 2: tmp = *(int *) data; break;
    case 3: tmp = *(long *) data; break;
    case 4: tmp = *(long long *) data; break;
    case 5: tmp = *(int64 *) data; break;
    default:
      return JSX_ReportException(cx, "Cannot convert C signed integer of unknown size to a javascript value");
  }
  if(!INT_FITS_IN_JSVAL(tmp)) return JS_NewNumberValue(cx, (jsdouble) tmp, rval);
  *rval = INT_TO_JSVAL(tmp);
  return JS_TRUE;
}


JSBool JsciTypeInt::JStoC(JSContext *cx, char *data, jsval v) {
  int tmpint;
  if(!this->JsToInt(cx, v, &tmpint)) return JS_FALSE;
  switch(this->size) {
    case 0:
      *(char *)data = tmpint;
      return JS_TRUE;
    case 1:
      *(short *)data = tmpint;
      return JS_TRUE;
    case 2:
      *(int *)data = tmpint;
      return JS_TRUE;
    case 3:
      *(long *)data = tmpint;
      return JS_TRUE;
    case 4:
      *(long long *)data = tmpint;
      return JS_TRUE;
    case 5:
      *(int64 *)data = tmpint;
      return JS_TRUE;
  }
  return JSX_ReportException(cx, "Cannot convert JS value to a C integer");
}


JSBool JsciTypeInt::JsToInt(JSContext *cx, jsval v, int *rv) {
  if(JSVAL_IS_INT(v)) { *rv = JSVAL_TO_INT(v); return JS_TRUE; }
  if(JSVAL_IS_DOUBLE(v)) { *rv = (int) *JSVAL_TO_DOUBLE(v); return JS_TRUE; }
  if(JSVAL_IS_BOOLEAN(v)) { *rv = v == JSVAL_TRUE ? 1 : 0; return JS_TRUE; }
  if(v == JSVAL_NULL) { *rv = 0; return JS_TRUE; }
  return JSX_ReportException(cx, "Cannot convert JS value of unrecognised type to a C integer");
}





JsciTypeUint::JsciTypeUint(int size, ffi_type ffit) : JsciTypeInt(size, ffit) {
}


JSBool JsciTypeUint::CtoJS(JSContext *cx, char *data, jsval *rval) {
  int tmp;
  // Return a number from an unsigned int (of various sizes)
  switch(this->size) {
    case 0: tmp = *(unsigned char *) data; break;
    case 1: tmp = *(unsigned short *) data; break;
    case 2: tmp = *(unsigned int *) data; break;
    case 3: tmp = *(unsigned long *) data; break;
    case 4: tmp = *(unsigned long long *) data; break;
    case 5: tmp = *(int64 *) data; break;
    default:
      return JSX_ReportException(cx, "Cannot convert C unsigned integer of unknown size to a javascript value");
  }
  if(!INT_FITS_IN_JSVAL(tmp)) return JS_NewNumberValue(cx, (jsdouble) tmp, rval);
  *rval = INT_TO_JSVAL(tmp);
  return JS_TRUE;
}





JsciTypeFunction::JsciTypeFunction(JsciType *returnType, int nParam) : returnType(returnType), nParam(nParam) {
  this->param = new JsciType*[this->nParam];
  this->cif.arg_types = 0;
}


JsciTypeFunction::~JsciTypeFunction() {
  if(this->param) delete this->param;
  if(this->cif.arg_types) delete this->cif.arg_types;
}


JSBool JsciTypeFunction::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JSX_ReportException(cx, "Pointer.prototype.$ cannot be used on pointers of type function.");
}


ffi_cif *JsciTypeFunction::GetCIF() {
  if(this->cif.arg_types) return &this->cif;

  this->cif.arg_types = new ffi_type*[this->nParam];
  for(int i = 0; i != this->nParam; ++i) {
    if(dynamic_cast<JsciTypeArray*>(this->param[i])) {
      this->cif.arg_types[i] = &ffi_type_pointer;
    } else {
      this->cif.arg_types[i] = this->param[i]->GetFFIType();
    }
  }
  ffi_prep_cif(&this->cif, FFI_DEFAULT_ABI, this->nParam, this->returnType->GetFFIType(), this->cif.arg_types);
  return &this->cif;
}


JSBool JsciTypeFunction::Call(JSContext *cx, void *cfunc, uintN argc, jsval *argv, jsval *rval) {
  if(this->nParam != argc) return JSX_ReportException(cx, "C function with %i parameters called with %i arguments", this->nParam, argc);

  int ok = JS_FALSE;

  int *argsizes = new int[argc];

  int arg_size = 0;
  for(uintN i = 0; i != this->nParam; ++i) {
    JsciType *t = this->param[i];
    int siz = argsizes[i] = t->SizeInBytes();
    if(!siz) { delete argsizes; return JS_FALSE; }
    arg_size += siz;
  }

  int retsize = this->returnType->SizeInBytes();

  void **argptr = new void*[argc];
  char *argbuf = new char[arg_size];
  char *retbuf = new char[retsize + 8]; // ffi overwrites a few bytes on some archs.

  if(arg_size) {
    char *ptr = argbuf;
    for(int i = 0; i != this->nParam; ++i) {
      JsciType *t = this->param[i];
      if(dynamic_cast<JsciTypeArray*>(t)) return JSX_ReportException(cx, "C function's parameter %i is of type array.  Make it a pointer, somehow", i);
      if(!t->JStoC(cx, ptr, argv[i])) goto failure;
      argptr[i] = ptr;
      ptr += argsizes[i];
    }
  }

  ffi_call(this->GetCIF(), FFI_FN(cfunc), (void *)retbuf, argptr);

  *rval=JSVAL_VOID;

  if(!dynamic_cast<JsciTypeVoid*>(this->returnType)) this->returnType->CtoJS(cx, retbuf, rval);

  ok = JS_TRUE;

 failure:
  delete argsizes;
  delete argptr;
  delete argbuf;
  delete retbuf;
  return ok;
}





JsciTypePointer::JsciTypePointer(JsciType *direct) : direct(direct) {
}


JSBool JsciTypePointer::CtoJS(JSContext *cx, char *data, jsval *rval) {
  if(*(void **)data == NULL) {
    *rval = JSVAL_NULL;
    return JS_TRUE;
  }
  return WrapPointer(cx, new JsciPointer(this->direct, *(void **) data), rval);
}


JSBool JsciTypePointer::JStoC(JSContext *cx, char *data, jsval v) {
  if(JSVAL_IS_NULL(v)) {
    *(void **)data = NULL;
    return JS_TRUE;
  }

  // Copy a string to a void* (same as char* in this context)
  if(JSVAL_IS_STRING(v)) {
    if(this->direct != gTypeVoid && this->direct != gTypeChar) return JSX_ReportException(cx, "Cannot convert JS string to C non-char non-void pointer type");
    *(char **)data = JS_GetStringBytes(JSVAL_TO_STRING(v));
    return JS_TRUE;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);

    if(JS_ObjectIsFunction(cx, obj)) {
      JsciTypeFunction* tf = dynamic_cast<JsciTypeFunction*>(this->direct);
      if(!tf) return JSX_ReportException(cx, "Cannot convert JS function to C non-function pointer type");
      JsciCallback *cb = JsciCallback::GetForFunction(cx, v, tf);
      *(void **)data = cb->codeptr();
      return JS_TRUE;
    }

    // Copy a pointer object to a type *
    JsciPointer *ptr = jsval_to_JsciPointer(cx, v);
    if(ptr) {
      *(void **)data = ptr->ptr;
      return JS_TRUE;
    }

    // Copy array elements to a variable array
    if(JS_IsArrayObject(cx, obj)) {
      jsuint size;
      JS_GetArrayLength(cx, obj, &size);
      int elemsize = this->direct->SizeInBytes();

      for(jsuint i = 0; i != size; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        if(!this->direct->JStoC(cx, *(char **)data + i * elemsize, tmp)) {
          delete data;
          return JS_FALSE;
        }
      }
      return JS_TRUE;
    }
  }

  return JSX_ReportException(cx, "Cannot convert JS value to C pointer type");
}


ffi_type *JsciTypePointer::GetFFIType() {
  return &ffi_type_pointer;
}


int JsciTypePointer::SizeInBytes() {
  return ffi_type_pointer.size;
}


int JsciTypePointer::AlignmentInBytes() {
  return ffi_type_pointer.alignment;
}





JsciTypeArray::JsciTypeArray(JsciType *type, int length) : member(type), length(length) {
}


JSBool JsciTypeArray::CtoJS(JSContext *cx, char *data, jsval *rval) {
  if(this->member == gTypeChar) {
    // Return a string from a char array
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, this->length));
    return JS_TRUE;
  }

  *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
  int fieldsize = this->SizeInBytes();
  JSObject *obj = JSVAL_TO_OBJECT(*rval);
  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  for(int i = 0; i != this->length; i++) {
    if(!this->member->CtoJS(cx, data + i * fieldsize, &tmp)) {
      JS_RemoveRoot(cx, &tmp);
      return JS_FALSE; // exception should already have been set
    }
    JS_SetElement(cx, obj, i, &tmp);
  }
  JS_RemoveRoot(cx, &tmp);
  return JS_TRUE;
}


JSBool JsciTypeArray::JStoC(JSContext *cx, char *data, jsval v) {
  if(JSVAL_IS_NULL(v)) {
    int size = this->SizeInBytes();
    memset(data, 0, size);
    return JS_TRUE;
  }

  if(JSVAL_IS_STRING(v)) {
      // Copy a string to a char array
      if(this->member != gTypeChar) return JSX_ReportException(cx, "Cannot convert JS string to a C array of non-chars");
      int size = JS_GetStringLength(JSVAL_TO_STRING(v));
      if(size < this->length) {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), size * sizeof(char));
        memset(*(char **)data + size, 0, (this->length - size) * sizeof(char));
      } else {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), this->length * sizeof(char));
      }
      return JS_TRUE;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);
    // Copy array elements to a fixed size array
    if(JS_IsArrayObject(cx, obj)) {
      int elsize = this->member->SizeInBytes();
      for(int i = 0; i != this->length; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        if(!this->member->JStoC(cx, data + elsize * i, tmp)) return JS_FALSE;
      }
      return JS_TRUE;
    }

    JsciPointer *ptr = jsval_to_JsciPointer(cx, v);
    if(ptr) {
      // Copy contents pointed to into array
      int size = this->SizeInBytes();
      memcpy(data, ptr->ptr, size);
      return JS_TRUE;
    }
  }

  return JSX_ReportException(cx, "Cannot convert JS value to C array");
}


int JsciTypeArray::SizeInBytes() {
  return this->length * this->member->SizeInBytes();
}


int JsciTypeArray::AlignmentInBytes() {
  return this->member->AlignmentInBytes();
}





JsciTypeBitfield::JsciTypeBitfield(JsciType *type, int length) : member(type), length(length) {
}


JSBool JsciTypeBitfield::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return this->member->CtoJS(cx, data, rval);
}


JSBool JsciTypeBitfield::JStoC(JSContext *cx, char *data, jsval v) {
  return JSX_ReportException(cx, "Cannot convert JS value to C bitfield value because the bitfield is not within a struct/union");
}


int JsciTypeBitfield::SizeInBits() {
  return this->length;
}


int JsciTypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}





JsciTypeStructUnion::JsciTypeStructUnion() : member(0), nMember(0), sizeOf(0) {
}


JsciTypeStructUnion::~JsciTypeStructUnion() {
  if(this->member) {
    // member names were strdup()'d earlier
    for(int i = 0; i != this->nMember; ++i) if(this->member[i].name) free(this->member[i].name);
    delete this->member;
  }
  if(this->ffiType.elements) delete this->ffiType.elements;
}


JSBool JsciTypeStructUnion::CtoJS(JSContext *cx, char *data, jsval *rval) {
  JSObject *obj = JS_NewObject(cx, 0, 0, 0);
  *rval = OBJECT_TO_JSVAL(obj);

  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  for(int i = 0; i != this->nMember; ++i) {
    JSX_SuMember mtype = this->member[i];
    JS_GetProperty(cx, obj, mtype.name, &tmp);
    if(!mtype.membertype->CtoJS(cx, data + mtype.offset / 8, &tmp)) {
      JS_RemoveRoot(cx, &tmp);
      return JS_FALSE; // the exception should already have been set
    }
    JsciTypeBitfield *tb = dynamic_cast<JsciTypeBitfield*>(mtype.membertype);
    if(tb) {
      int length = tb->length;
      int offset = mtype.offset % 8;
      int mask = ~(-1 << length);
      tmp = INT_TO_JSVAL((JSVAL_TO_INT(tmp) >> offset) & mask);
    }
    JS_SetProperty(cx, obj, mtype.name, &tmp);
  }
  JS_RemoveRoot(cx, &tmp);
  return JS_TRUE;
}


JSBool JsciTypeStructUnion::JStoC(JSContext *cx, char *data, jsval v) {
  // Copy object elements to a struct or union
  if(v == JSVAL_NULL) {
    int size = this->SizeInBytes();
    memset(data, 0, size);
    return JS_TRUE;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);
    for(int i = 0; i != this->nMember; ++i) {
      jsval tmp;
      JS_GetProperty(cx, obj, this->member[i].name, &tmp);
      JsciType *t = this->member[i].membertype;
      JsciTypeBitfield *tb = dynamic_cast<JsciTypeBitfield*>(t);
      if(tb) {
        int length = tb->length;
        int offset = this->member[i].offset % 8;
        int mask = ~(-1 << length);
        int imask = ~(imask << offset); // xxx imask is undefined!
        int tmpint, tmpint2;
        if(!t->JStoC(cx, (char *) &tmpint, tmp)) return JS_FALSE;
        int thissize = t->SizeInBytes();
        memcpy((char *) &tmpint2, data + this->member[i].offset / 8, thissize);
        tmpint = (tmpint2 & imask) | ((tmpint & mask) << offset);
        memcpy(data + this->member[i].offset / 8, (char *) &tmpint, thissize);
      } else {
        if(!t->JStoC(cx, data + this->member[i].offset / 8, tmp)) return JS_FALSE;
      }
    }
    return JS_TRUE;
  }

  return JSX_ReportException(cx, "Cannot convert JS value to a C struct/union");
}


int JsciTypeStructUnion::SizeInBytes() {
  int align = this->AlignmentInBytes();
  return (((this->sizeOf + 7) / 8 + align - 1) / align) * align;
}


int JsciTypeStructUnion::AlignmentInBytes() {
  int ret = 0;
  for(int i = 0; i != this->nMember; ++i) {
    int thisalign = this->member[i].membertype->AlignmentInBytes();
    if(thisalign > ret) ret = thisalign;
  }
  return ret;
}


JSBool JsciTypeStructUnion::ReplaceMembers(JSContext *cx, JSObject *obj, int nMember, jsval *members) {
  this->nMember = nMember;
  this->member = new JSX_SuMember[nMember];

  jsval tmp;
  for(int i = 0; i != nMember; ++i) {
    if(!JSVAL_IS_OBJECT(members[i]) || JSVAL_IS_NULL(members[i])) goto failure;

    JSObject *mobj = JSVAL_TO_OBJECT(members[i]);
    JSX_SuMember *m = this->member + i;
    JS_GetProperty(cx, mobj, "name", &tmp);
    if(tmp == JSVAL_VOID || !JSVAL_IS_STRING(tmp)) return JSX_ReportException(cx, "Wrong or missing 'name' property in member type object");
    m->name = strdup(JS_GetStringBytes(JSVAL_TO_STRING(tmp)));
    JS_GetProperty(cx, mobj, "type", &tmp);
    m->membertype = jsval_to_JsciType(cx, tmp);
    if(!m->membertype) return JSX_ReportException(cx, "Struct/union-member descriptor doesn't have a .type property of type Type");

    // this is probably just to save the Type instances from GC, and thus the JsciType's from being free()'d
    JS_DefineElement(cx, obj, i, members[i], 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }
  if(!this->SetSizeAndAligments(cx)) goto failure;
  return JS_TRUE;

 failure:
  delete this->member;
  this->member = 0;
  this->nMember = 0;
  return JS_FALSE;
}





JsciTypeStruct::JsciTypeStruct() : JsciTypeStructUnion() {
}


ffi_type *JsciTypeStruct::GetFFIType() {
  if(!this->ffiType.elements) {
    int nmember = 0;
    int bitsused = 0;
    for(int i = 0; i != this->nMember; ++i) {
      int al = 1;
      JsciType *memb = this->member[i].membertype;
      JsciTypeArray *ta;
      while((ta = dynamic_cast<JsciTypeArray*>(memb))) {
        al *= ta->length;
        memb = ta->member;
      }
      JsciTypeBitfield *tb = dynamic_cast<JsciTypeBitfield*>(memb);
      if(tb) {
        int length = tb->length;
        if(bitsused && bitsused + length < 8) {
          al = 0;
        } else {
          al = (bitsused + length) / 8;
          bitsused = (bitsused + length) % 8;
        }
      } else {
        bitsused = 0;
      }
      nmember += al;
    }

    this->ffiType.elements = new _ffi_type*[nmember + 1];

    // must specify size and alignment because bitfields introduce alignment requirements which are not reflected by the ffi members.
    this->ffiType.size = this->SizeInBytes();
    this->ffiType.alignment = this->AlignmentInBytes();
    this->ffiType.type = FFI_TYPE_STRUCT;

    bitsused = 0;
    nmember = 0;
    for(int i = 0; i != this->nMember; ++i) {
      int al = 1;
      ffi_type *t;
      JsciType *memb = this->member[i].membertype;
      JsciTypeArray *ta;
      while((ta = dynamic_cast<JsciTypeArray*>(memb))) {
        al *= ta->length;
        memb = ta->member;
      }
      JsciTypeBitfield *tb = dynamic_cast<JsciTypeBitfield*>(memb);
      if(tb) {
        int length = tb->length;
        if(bitsused && bitsused + length < 8) {
          al = 0;
        } else {
          al = (bitsused + length) / 8;
          bitsused = (bitsused + length) % 8;
        }
        t = &ffi_type_uchar;
      } else {
        bitsused = 0;
        t = memb->GetFFIType();
      }
      for(int j = 0; j < al; ++j) this->ffiType.elements[nmember++] = t;
    }
    this->ffiType.elements[nmember] = NULL;
  }

  return &this->ffiType;
}


JSBool JsciTypeStruct::SetSizeAndAligments(JSContext *cx) {
  for(int i = 0; i != this->nMember; ++i) {
    int thisalign = this->member[i].membertype->AlignmentInBits();
    if(thisalign == 0) return JSX_ReportException(cx, "Division by zero");
    this->sizeOf += (thisalign - this->sizeOf % thisalign) % thisalign;
    this->member[i].offset = this->sizeOf;
    this->sizeOf += this->member[i].membertype->SizeInBits();
  }
  return JS_TRUE;
}





JsciTypeUnion::JsciTypeUnion() : JsciTypeStructUnion() {
}


JSBool JsciTypeUnion::SetSizeAndAligments(JSContext *cx) {
  for(int i = 0; i != this->nMember; ++i) {
    this->member[i].offset = 0;
    int sz = this->member[i].membertype->SizeInBits();
    if(sz > this->sizeOf) this->sizeOf = sz;
  }
  return JS_TRUE;
}

#include <stdlib.h>
#include <string.h>
#include "jsci.h"


JsciTypeStructUnion::JsciTypeStructUnion() : JsciType(SUTYPE), member(0), nMember(0), sizeOf(0) {
}


JsciTypeStructUnion::~JsciTypeStructUnion() {
  if(this->member) {
    // member names were strdup()'d earlier
    for(int i = 0; i != this->nMember; ++i) if(this->member[i].name) free(this->member[i].name);
    delete this->member;
  }
  if(this->ffiType.elements) delete this->ffiType.elements;
}


int JsciTypeStructUnion::CtoJS(JSContext *cx, char *data, jsval *rval) {
  JSObject *obj = JS_NewObject(cx, 0, 0, 0);
  *rval = OBJECT_TO_JSVAL(obj);

  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  for(int i = 0; i != this->nMember; ++i) {
    JSX_SuMember mtype = this->member[i];
    JS_GetProperty(cx, obj, mtype.name, &tmp);
    int ok = mtype.membertype->CtoJS(cx, data + mtype.offset / 8, &tmp);
    if(!ok) {
      JS_RemoveRoot(cx, &tmp);
      return 0; // the exception should already have been set
    }
    if(mtype.membertype->type == BITFIELDTYPE) {
      int length = ((JsciTypeBitfield *) mtype.membertype)->length;
      int offset = mtype.offset % 8;
      int mask = ~(-1 << length);
      tmp = INT_TO_JSVAL((JSVAL_TO_INT(tmp) >> offset) & mask);
    }
    JS_SetProperty(cx, obj, mtype.name, &tmp);
  }
  JS_RemoveRoot(cx, &tmp);
  return 1;
}


int JsciTypeStructUnion::JStoC(JSContext *cx, char *data, jsval v, int will_clean) {
  // Copy object elements to a struct or union
  if(v == JSVAL_NULL) {
    int size = this->SizeInBytes();
    memset(data, 0, size);
    return size;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);
    for(int i = 0; i != this->nMember; ++i) {
      jsval tmp;
      JS_GetProperty(cx, obj, this->member[i].name, &tmp);
      JsciType *t = this->member[i].membertype;
      if(t->type == BITFIELDTYPE) {
        int length = ((JsciTypeBitfield *) t)->length;
        int offset = this->member[i].offset % 8;
        int mask = ~(-1 << length);
        int imask = ~(imask << offset); // xxx imask is undefined!
        int tmpint, tmpint2;
        if(!t->JStoC(cx, (char *) &tmpint, tmp, will_clean)) return 0;
        int thissize = t->SizeInBytes();
        memcpy((char *) &tmpint2, data + this->member[i].offset / 8, thissize);
        tmpint = (tmpint2 & imask) | ((tmpint & mask) << offset);
        memcpy(data + this->member[i].offset / 8, (char *) &tmpint, thissize);
      } else {
        if(!t->JStoC(cx, data + this->member[i].offset / 8, tmp, will_clean)) return 0;
      }
    }
    return this->SizeInBytes();
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


JSBool JsciTypeStructUnion::ContainsPointer() {
  for(int i = 0; i != this->nMember; ++i)
    if(this->member[i].membertype->ContainsPointer())
      return JS_TRUE;
  return JS_FALSE;
}


JSBool JsciTypeStructUnion::ReplaceMembers(JSContext *cx, JSObject *obj, int nMember, jsval *members) {
  this->nMember = nMember;
  this->member = new JSX_SuMember[nMember];

  for(int i = 0; i != nMember; ++i) {
    if(!JSVAL_IS_OBJECT(members[i]) || JSVAL_IS_NULL(members[i])) goto failure;
    if(!JSX_InitMemberType(cx, this->member + i, JSVAL_TO_OBJECT(members[i]))) goto failure;
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

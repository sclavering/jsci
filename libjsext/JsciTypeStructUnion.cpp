#include <string.h>
#include "jsci.h"


int JSX_TypeStructUnion::SizeInBytes() {
  int align = JSX_TypeAlign(this);
  return (((this->sizeOf + 7) / 8 + align - 1) / align) * align;
}


JSBool JSX_TypeStructUnion::ReplaceMembers(JSContext *cx, JSObject *obj, int nMember, jsval *members) {
  this->nMember = nMember;
  this->member = new JSX_SuMember[nMember];
  memset(this->member, 0, sizeof(JSX_SuMember) * nMember);

  for(int i = 0; i != nMember; ++i) {
    if(!JSVAL_IS_OBJECT(members[i]) || JSVAL_IS_NULL(members[i])) goto failure;
    if(!JSX_InitMemberType(cx, this->member + i, JSVAL_TO_OBJECT(members[i]))) goto failure;
    // this is probably just to save the Type instances from GC, and thus the JSX_Type's from being free()'d
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

#include "jsci.h"


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

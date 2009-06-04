#include "jsci.h"


ffi_type *JSX_TypeStructUnion::GetFFIType(JSContext *cx) {
  if(this->type == UNIONTYPE) return 0;

  if(!this->ffiType.elements) {
    int nmember = 0;
    int bitsused = 0;
    for(int i = 0; i != this->nMember; ++i) {
      int al = 1;
      JSX_Type *memb = this->member[i].membertype;
      while(memb->type == ARRAYTYPE) {
        al *= ((JSX_TypeArray *) memb)->length;
        memb = ((JSX_TypeArray *) memb)->member;
      }
      if(memb->type == BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) memb)->length;
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
    this->ffiType.size = JSX_TypeSize(this);
    this->ffiType.alignment = JSX_TypeAlign(this);
    this->ffiType.type = FFI_TYPE_STRUCT;

    bitsused = 0;
    nmember = 0;
    for(int i = 0; i != this->nMember; ++i) {
      int al = 1;
      ffi_type *t;
      JSX_Type *memb = this->member[i].membertype;
      while(memb->type == ARRAYTYPE) {
        al *= ((JSX_TypeArray *) memb)->length;
        memb = ((JSX_TypeArray *) memb)->member;
      }
      if(memb->type == BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) memb)->length;
        if(bitsused && bitsused + length < 8) {
          al = 0;
        } else {
          al = (bitsused + length) / 8;
          bitsused = (bitsused + length) % 8;
        }
        t = &ffi_type_uchar;
      } else {
        bitsused = 0;
        t = memb->GetFFIType(cx);
      }
      for(int j = 0; j < al; ++j) this->ffiType.elements[nmember++] = t;
    }
    this->ffiType.elements[nmember] = NULL;
  }

  return &this->ffiType;
}

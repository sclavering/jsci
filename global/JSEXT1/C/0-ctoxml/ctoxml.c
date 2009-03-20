/*
* Copyright (c) 2006, Berges Allmenndigitale Rådgivningstjeneste
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Berges Allmenndigitale Rådgivningstjeneste nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE BERGES AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE BERGES AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ctoxml.h"
#include <string.h>
#include "stringhash.h"
#include <stdlib.h>

stringhash *ctoxml_typedefs;


void ctoxml_init(void) {
  ctoxml_typedefs=stringhash_new();
  ctoxml_filename=0;
}

void ctoxml_end(void) {
  stringhash_destroy(ctoxml_typedefs);
}

void xmlesc(char *str) {
  char buf[2]={0,0};
  while (*str) {
    switch(*str) {
    case '<': PUTS("&lt;"); break;
    case '>': PUTS("&gt;"); break;
    case '&': PUTS("&amp;"); break;
    default: buf[0]=*str; PUTS(buf); break;
    }
    str++;
  }
}


static void deftypes(struct Xml *e, struct Xml *td) {
	// 2. find all ident tags and insert them into typedefs container

	struct Xml *i=e;
	do {
		if (i->text && strcmp(i->tag,"id")==0) {
                  struct Xml *shallowcopy=malloc(sizeof(struct Xml));
                  memcpy(shallowcopy,td,sizeof(struct Xml));
		  stringhash_remove(ctoxml_typedefs, i->text);
		  stringhash_insert(ctoxml_typedefs, strdup(i->text), shallowcopy);
		}
		if (i->inner && (strcmp(i->tag,"ptr")==0 ||
				 strcmp(i->tag,"ix")==0 ||
				 strcmp(i->tag,"p")==0 ||
				 strcmp(i->tag,"fd")==0
				 )) deftypes(i->inner, td);
		i=i->next;
	} while (i!=e);
}

void ctoxml_deftype_to_ident(struct Xml *e) {
  struct Xml *ptr=e->inner->last;
  struct Xml *ptrstop=e->inner;

  //  if (e->inner==e->inner->next) return; (should be covered by next test)

  while (strcmp(ptrstop->tag,"const")==0 ||
	 strcmp(ptrstop->tag,"typedef")==0 ||
	 strcmp(ptrstop->tag,"volatile")==0) ptrstop=ptrstop->next;

  if (ptrstop==ptr) return; // only one token, must be dt

  while ((strcmp(ptr->tag,"id")==0 ||
	  strcmp(ptr->tag,"ptr")==0 ||
	  strcmp(ptr->tag,"ix")==0) &&
	 ptr->last!=ptrstop) ptr=ptr->last;
  
  if (strcmp(ptr->tag,"dt")==0) ptr->tag="id";

}

void ctoxml_typedef(struct Xml *e) {
  // check if identifier is replaced with a deftype,
  //    in which case it is a redefinition

  ctoxml_deftype_to_ident(e);

  deftypes(e->inner,e);
}


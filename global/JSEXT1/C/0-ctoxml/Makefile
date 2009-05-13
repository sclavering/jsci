# Copyright (c) 2006, Berges Allmenndigitale Rådgivningstjeneste
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Berges Allmenndigitale Rådgivningstjeneste nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE BERGES AND CONTRIBUTORS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE BERGES AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


YFLAGS	= -dv
CFLAGS	= -O3 -fPIC
LFLAGS	=
LDFLAGS	= 

TARGETS	= libctoxml.so

all:	$(TARGETS)

c_scan.h:	c_scan.c

c_scan.c:	c_scan.l
	$(LEX) -Pctoxml_c -t --header-file=c_scan.h c_scan.l > c_scan.c

c_scan.o	: c_gram.h

c_gram.c:	c_gram.y
	$(YACC) -p ctoxml_c -dv -o c_gram.c c_gram.y

LIBCTOXMLOBJ	= c_gram.o c_scan.o ctoxml.o xml.o stringhash.o hashtable.o libctoxml.o strbuf.o hashtable_itr.o

libctoxml.so:	$(LIBCTOXMLOBJ)
	$(CC) $(LIBCTOXMLOBJ) -shared -o libctoxml.so


clean	:
	rm -f y.output *.o $(TARGETS) c_gram.output lex.yy.c *~ y.tab.h *.suo
#	rm -f c_scan.c c_gram.c

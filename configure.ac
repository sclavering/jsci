#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

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

AC_PREREQ(2.59)
AC_INIT(jsext, m4_esyscmd([cat RELEASE]), bugreport@jsext.net)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_LEX
AC_PROG_YACC

AC_PREFIX_DEFAULT([/usr])
# Checks for libraries.

AC_CHECK_LIB([dl], [dlopen])

AC_CHECK_LIB([m], [sin])

#AC_CHECK_LIB([js], [JS_NewContext],,
#		   [AC_MSG_ERROR([Missing spidermonkey])])

[JSAPI_FLAGS="-D XP_UNIX"; THREADSAFE=""]

# Check for threadsafe spidermonkey
AC_ARG_ENABLE([threadsafe], [  --enable-threadsafe    Compile with thread support], 
		   [JSAPI_FLAGS="-D XP_UNIX -D JS_THREADSAFE"; THREADSAFE="JS_THREADSAFE=1"])

#AC_CHECK_HEADER([js/jsapi.h],,
#			[AC_MSG_ERROR([Missing spidermonkey header files])])

#JSAPI_H_DIR=/usr/include/jsapi


AC_SUBST(JSAPI_FLAGS)
#AC_SUBST(JSAPI_H_DIR)

AC_SUBST(THREADSAFE)
AC_SUBST(libdir)
AC_SUBST(mandir)
AC_SUBST(bindir)
AC_SUBST(prefix)
AC_SUBST(sysconfdir)
AC_SUBST(exec_prefix)
AC_SUBST(PACKAGE_VERSION)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([readline/readline.h readline/history.h],,
		[AC_MSG_ERROR([Missing readline])])
AC_CHECK_HEADERS([ffi.h],,
		[AC_MSG_ERROR([Missing libffi])])
AC_CHECK_HEADERS([fcntl.h inttypes.h stddef.h stdlib.h string.h unistd.h],,
		[AC_MSG_ERROR([Missing standard header file])])
#AC_CHECK_HEADERS([jsapi/jsapi.h jsapi/jsfun.h],,
#		[AC_MSG_ERROR([Missing jsapi header file])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset putenv setenv strchr strcspn strdup strspn strstr])

AC_CONFIG_FILES([Makefile
		 libjsext/Makefile
                 global/Makefile
                 global/JSEXT1/Makefile])
AC_OUTPUT

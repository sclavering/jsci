#ifndef LIBJSEXT_H
#define LIBJSEXT_H

/*
  Initializes jsext with ini_file from
  global_path if not null
  environment variable JSEXT_INI if set
  compiler option $prefix/lib/jsext-$version.js
 */

#include <jsapi.h>

JSBool JSX_init(JSContext *cx, JSObject *obj, char *ini_file, jsval *rval);

#endif

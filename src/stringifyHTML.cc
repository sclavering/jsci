#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "jscntxt.h" // for js_ReportAllocationOverflow
#include "jsscan.h"
#include "jsstr.h"
#include "jsxml.h"


static JSBool stringifyHTML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSString *EscapeElementValue(JSContext *cx, JSStringBuffer *sb, JSString *str);
static JSString *EscapeAttributeValue(JSContext *cx, JSStringBuffer *sb, JSString *str);
static JSString *XMLToXMLString(JSContext *cx, JSXML *xml);
static void AppendAllAttributes(JSContext *cx, JSXML *xml, JSStringBuffer *sb);
static void AppendAllChildren(JSContext *cx, JSXML *xml, JSStringBuffer *sb);
static void AppendAttributeValue(JSContext *cx, JSStringBuffer *sb, JSString *valstr);


// the objects keys are used as a set.  the members are provided from JS code (it's easier)
static JSObject *empty_tag_names = 0;
static JSObject *boolean_attribute_names = 0;
static JSObject *boolean_attribute_falsey_values = 0;


static inline JSBool is_in_set(JSContext *cx, JSString *str, JSObject *set) {
  jsval tmp = JSVAL_VOID;
  return JS_LookupUCProperty(cx, set, JS_GetStringChars(str), JS_GetStringLength(str), &tmp) && tmp != JSVAL_VOID;
}


static void XMLArrayCursorInit(JSXMLArrayCursor *cursor, JSXMLArray *array){
  JSXMLArrayCursor *next;
  cursor->array = array;
  cursor->index = 0;
  next = cursor->next = array->cursors;
  if(next) next->prevp = &cursor->next;
  cursor->prevp = &array->cursors;
  array->cursors = cursor;
  cursor->root = NULL;
}


static void XMLArrayCursorFinish(JSXMLArrayCursor *cursor) {
  JSXMLArrayCursor *next;
  if(!cursor->array) return;
  next = cursor->next;
  if(next) next->prevp = cursor->prevp;
  *cursor->prevp = next;
  cursor->array = NULL;
}


static JSXML *XMLArrayCursorNext(JSXMLArrayCursor *cursor) {
  JSXMLArray *array = cursor->array;
  if(!array || cursor->index >= array->length) return NULL;
  return (JSXML*) (cursor->root = array->vector[cursor->index++]);
}


static JSString *XMLToXMLString(JSContext *cx, JSXML *xml) {
  JSStringBuffer sb;
  js_InitStringBuffer(&sb);

  JSString *str = NULL;

  switch (xml->xml_class) {
    case JSXML_CLASS_TEXT:
      return EscapeElementValue(cx, &sb, xml->xml_value);

    case JSXML_CLASS_ATTRIBUTE:
      return EscapeAttributeValue(cx, &sb, xml->xml_value);

    case JSXML_CLASS_LIST: {
      JSXMLArrayCursor cursor;
      XMLArrayCursorInit(&cursor, &xml->xml_kids);
      JSXML *kid;
      while((kid = XMLArrayCursorNext(&cursor))) {
        JSString *kidstr = XMLToXMLString(cx, kid);
        if (!kidstr) break;
        js_AppendJSString(&sb, kidstr);
      }
      XMLArrayCursorFinish(&cursor);
      if (kid) goto list_out;

      if (!sb.base) return JSVAL_TO_STRING(JS_GetEmptyStringValue(cx));

      if (!STRING_BUFFER_OK(&sb)) {
          JS_ReportOutOfMemory(cx);
          return NULL;
      }

      str = js_NewString(cx, sb.base, STRING_BUFFER_OFFSET(&sb));
    list_out:
      if (!str && STRING_BUFFER_OK(&sb)) js_FinishStringBuffer(&sb);
      return str;
    }

    case JSXML_CLASS_COMMENT:
      return 0;

    // So that stringifyHTML() callers have some way to get raw HTML (e.g. from a database) included within an E4X template
    case JSXML_CLASS_PROCESSING_INSTRUCTION:
      // xxx should check ->name->localName first, for "jsx_raw_html" or similar
      return xml->xml_value;
  }

  /* After this point, control must flow through label out: to exit. */
  if(!JS_EnterLocalRootScope(cx)) return NULL;

  js_AppendChar(&sb, '<');
  js_AppendJSString(&sb, xml->name->localName);

  AppendAllAttributes(cx, xml, &sb);

  js_AppendChar(&sb, '>');
  if(!is_in_set(cx, xml->name->localName, empty_tag_names)) {
    AppendAllChildren(cx, xml, &sb);
    js_AppendCString(&sb, "</");
    js_AppendJSString(&sb, xml->name->localName);
    js_AppendChar(&sb, '>');
  }

  if (!STRING_BUFFER_OK(&sb)) {
    JS_ReportOutOfMemory(cx);
    goto out;
  }

  str = js_NewString(cx, sb.base, STRING_BUFFER_OFFSET(&sb));
out:
  JS_LeaveLocalRootScopeWithResult(cx, STRING_TO_JSVAL(str));
  if(!str && STRING_BUFFER_OK(&sb)) js_FinishStringBuffer(&sb);
  return str;
}


static void AppendAllAttributes(JSContext *cx, JSXML *xml, JSStringBuffer *sb) {
  JSXMLArrayCursor cursor;
  XMLArrayCursorInit(&cursor, &xml->xml_attrs);
  JSXML *attr;
  while((attr = XMLArrayCursorNext(&cursor))) {
    JSBool isbool = is_in_set(cx, attr->name->localName, boolean_attribute_names);
    JSBool isfalsey = isbool && is_in_set(cx, attr->xml_value, boolean_attribute_falsey_values);
    if(!isbool || !isfalsey) {
      js_AppendChar(sb, ' ');
      js_AppendJSString(sb, attr->name->localName);
    }
    if(!isbool) AppendAttributeValue(cx, sb, attr->xml_value);
  }
  XMLArrayCursorFinish(&cursor);
}


static void AppendAllChildren(JSContext *cx, JSXML *xml, JSStringBuffer *sb) {
  JSXMLArrayCursor cursor;
  XMLArrayCursorInit(&cursor, &xml->xml_kids);
  JSXML *kid;
  while((kid = XMLArrayCursorNext(&cursor))) {
    JSString *kidstr = XMLToXMLString(cx, kid);
    if(!kidstr) break;
    js_AppendJSString(sb, kidstr);
  }
  XMLArrayCursorFinish(&cursor);
}


// This function takes ownership of sb->base, if sb is non-null, in all cases of success or failure.
static JSString *EscapeElementValue(JSContext *cx, JSStringBuffer *sb, JSString *str) {
  size_t length;
  const jschar *cp, *start, *end;
  jschar c;

  JSSTRING_CHARS_AND_LENGTH(str, start, length);
  size_t newlength = length;
  for (cp = start, end = cp + length; cp < end; cp++) {
    c = *cp;
    if (c == '<' || c == '>')
      newlength += 3;
    else if (c == '&')
      newlength += 4;

    if (newlength < length) {
      js_ReportAllocationOverflow(cx);
      return NULL;
    }
  }
  if(STRING_BUFFER_OFFSET(sb) != 0 || newlength > length) {
    if (!sb->grow(sb, newlength)) {
      JS_ReportOutOfMemory(cx);
      return NULL;
    }
    for (cp = start; cp < end; cp++) {
      c = *cp;
      if (c == '<')
        js_AppendCString(sb, js_lt_entity_str);
      else if (c == '>')
        js_AppendCString(sb, js_gt_entity_str);
      else if (c == '&')
        js_AppendCString(sb, js_amp_entity_str);
      else
        js_AppendChar(sb, c);
    }
    JS_ASSERT(STRING_BUFFER_OK(sb));
    str = js_NewString(cx, sb->base, STRING_BUFFER_OFFSET(sb));
    if (!str) js_FinishStringBuffer(sb);
  }
  return str;
}


// This function takes ownership of sb->base, if sb is non-null, in all cases.
static JSString *EscapeAttributeValue(JSContext *cx, JSStringBuffer *sb, JSString *str) {
  size_t length;
  const jschar *cp, *start, *end;
  jschar c;

  JSSTRING_CHARS_AND_LENGTH(str, start, length);
  size_t newlength = length + 2; // +2 for the quote marks
  for (cp = start, end = cp + length; cp < end; cp++) {
    c = *cp;
    if (c == '"')
      newlength += 5;
    else if (c == '<')
      newlength += 3;
    else if (c == '&')
      newlength += 4;

    if (newlength < length) {
      js_ReportAllocationOverflow(cx);
      return NULL;
    }
  }
  if(STRING_BUFFER_OFFSET(sb) != 0 || newlength > length) {
    if (!sb->grow(sb, newlength)) {
      JS_ReportOutOfMemory(cx);
      return NULL;
    }
    js_AppendChar(sb, '"');
    for (cp = start; cp < end; cp++) {
      c = *cp;
      if (c == '"')
        js_AppendCString(sb, js_quot_entity_str);
      else if (c == '<')
        js_AppendCString(sb, js_lt_entity_str);
      else if (c == '&')
        js_AppendCString(sb, js_amp_entity_str);
      else
        js_AppendChar(sb, c);
    }
    js_AppendChar(sb, '"');
    JS_ASSERT(STRING_BUFFER_OK(sb));
    str = js_NewString(cx, sb->base, STRING_BUFFER_OFFSET(sb));
    if (!str) js_FinishStringBuffer(sb);
  }
  return str;
}


static void AppendAttributeValue(JSContext *cx, JSStringBuffer *sb, JSString *valstr) {
  js_AppendChar(sb, '=');
  valstr = js_EscapeAttributeValue(cx, valstr, JS_TRUE);
  if (!valstr) {
    if (STRING_BUFFER_OK(sb)) {
      free(sb->base);
      sb->base = STRING_BUFFER_ERROR_BASE;
    }
    return;
  }
  js_AppendJSString(sb, valstr);
}


static JSBool stringifyHTML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval v = argv[0];
  if(!JSVAL_IS_OBJECT(v) || JSVAL_IS_NULL(v)) return JSX_ReportException(cx, "stringifyHTML(): argument should be an XML/XMLList instance, but isn't even an object");
  JSObject *o = JSVAL_TO_OBJECT(v);
  if(!OBJECT_IS_XML(cx, o)) return JSX_ReportException(cx, "stringifyHTML: argument should be an XML/XMLList instance, but is of a different object type");
  JSXML *xml = (JSXML *) JS_GetPrivate(cx, o);
  JSString *str = XMLToXMLString(cx, xml);
  if(str) *rval = STRING_TO_JSVAL(str);
  else *rval = JS_GetEmptyStringValue(cx);
  return JS_TRUE;
}


jsval make_stringifyHTML(JSContext *cx) {
  JSFunction *f = JS_NewFunction(cx, stringifyHTML, 1, 0, 0, "stringifyHTML");
  if(!f) return JSVAL_VOID;
  JSObject *o = JS_GetFunctionObject(f);
  empty_tag_names = JS_NewObject(cx, 0, 0, 0);
  if(!JS_DefineProperty(cx, o, "empty_tag_names", OBJECT_TO_JSVAL(empty_tag_names), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT)) return JSVAL_VOID;
  boolean_attribute_names = JS_NewObject(cx, 0, 0, 0);
  if(!JS_DefineProperty(cx, o, "boolean_attribute_names", OBJECT_TO_JSVAL(boolean_attribute_names), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT)) return JSVAL_VOID;
  boolean_attribute_falsey_values = JS_NewObject(cx, 0, 0, 0);
  if(!JS_DefineProperty(cx, o, "boolean_attribute_falsey_values", OBJECT_TO_JSVAL(boolean_attribute_falsey_values), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT)) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(o);
}

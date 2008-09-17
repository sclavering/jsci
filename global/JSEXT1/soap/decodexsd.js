/*

      val = decodexsd( type, element )

  Converts an element to a JavaScript value using an
  xsd type name to guide the conversion.

  ### Arguments ###

  * _type_: A string, like "int", "string" etc.
  * _element_ An XML node.

  ### Return value ###

  A string, number, boolean, date object or other values.
  
 */

function(type, element) {
  switch(type) {
  case 'string':
    return String(element);
  case 'anyURI':
    return $parent.http.decodeURI(String(element));
  case 'boolean': 
    return Boolean(element);
  case 'float': 
    return Number(element);
  case 'integer': 
    return Number(element);
  case 'int': 
    return Number(element);
  case 'nonPositiveInteger': 
    return Number(element);
  case 'negativeInteger': 
    return Number(element);
  case 'long': 
    return Number(element);
  case 'short': 
    return Number(element);
  case 'byte': 
    return Number(element);
  case 'nonNegativeInteger': 
    return Number(element);
  case 'unsignedLong': 
    return Number(element);
  case 'unsignedInt': 
    return Number(element);
  case 'unsignedShort': 
    return Number(element);
  case 'unsignedByte': 
    return Number(element);
  case 'positiveInteger': 
    return Number(element);
  case 'double': 
    return Number(element);
  case 'decimal': 
    return Number(element);
  case 'datetime': 
    return new Date(element);
  case 'time': 
    throw("unimplemented");
  case 'base64Binary': 
    return $parent.decodeBase64(String(element));
  } 
}

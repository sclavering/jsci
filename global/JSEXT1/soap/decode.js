/*
         val = decode(xml, schema [, typedefs])

     Converts from xml to JS values
*/

function(xml, schema, typedefs) {

  var conv={};
  conv[xsd]=decodexsd;

  var targetNamespace=xml.namespace();

  if (typedefs) {
    conv[typedefs.@targetNamespace]=convtypes;
    targetNamespace=typedefs.@targetNamespace;
  }

  return inner(xml, schema);

  function inner(xml, schema) {

    if (schema.@type.length()) {

      var type=toQName(schema.@type);
      return conv[type.uri](type.localName, xml);

    } else {

      return complex(xml, schema.xsd::complexType);
    }

  }

  function complex(xml, complexType) {
    var ret={};
      
    for each (var member in complexType.xsd::sequence.*) {
      var element=xml[toQName(member.@name, targetNamespace)];

      if (element.length()==0)
	continue;

      if ((member.@maxOccurs=="1" || member.@maxOccurs.length()==0) && element.length()==1) {
	ret[member.@name]=inner(element, member);
      } else {
	var array=[];
	for each (var subelm in element) {
	  array.push(inner(subelm, member));
	}
	ret[member.@name]=array;
      }
      
    }
    
    return ret;
  }

  function convtypes(type, xml) {
    var t=typedefs.xsd::complexType.(@name==type);
    return complex(xml, t);
  }

}

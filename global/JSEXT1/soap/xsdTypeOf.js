/*

      val = xsdTypeOf (sample)

  Returns the xsd type of a JavaScript sample value as a string.
  If the value has a complex type, an XML Schema fragment is returned
  as an [[XML]] object.

 */

function(sample) {
  switch(typeof sample) {

    case 'string':
      return "string";

    case 'number':
      if (sample==Math.floor(sample))
	return "int";
      else
	return "double";

    case 'boolean':
      return "boolean";

    case 'object':

      if (sample instanceof Date) {

	return "datetime";

      } else {

        var members=new XMLList;
        for (var i in sample) {
	  var type = arguments.callee(sample[i]);
          if (typeof type === "string") {
	    members += <element minOccurs="1" maxOccurs="1" name={i} type={"xsd:"+type} />
          } else {
	    members += <element minOccurs="1" maxOccurs="1" name={i} >
                         {type}
                       </element>
	  }
        }
        return <complexType xmlns={xsd} xmlns:xsd={xsd} >
                 <sequence>
                   {members}
                 </sequence>
	       </complexType>
      }
  }
}

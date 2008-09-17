  /*
        array = http.getFormDataByNames(names [, data1 [, data2, ...]])

    Returns an array of form data in the same order as the names given in the _names_ array.
    _data1_ elements take precedence over _data2_ elements by the same name.

    Data elements with numerical names ("0", "1" etc) are assigned
    to the corresponding element number in the return array.

    ---
    
    **Example**

    ---

        param = getFormData(["c","b","a"], {a:42, b:50}, {b:60, c:70})

    Because of the ordering of the elements in the array, the _param_
    array will contain the following properties afterwards:

        [70,50,42]

    ---

   */

function(names) {
  var ret=[];
  var hasOwnProperty=Object.prototype.hasOwnProperty;

  for (var i=0; i<names.length; i++) {
    for (var j=1; j<arguments.length; j++)
      if (hasOwnProperty.call(arguments[j], names[i]))
	ret[i]=arguments[j][names[i]];
  }
 
  for (var i=0; ; i++) {
    for (var j=1; j<arguments.length; j++)
      if (hasOwnProperty.call(arguments[j], i)) {
	ret[i]=arguments[j][i];
	break;
      }
    if (j==arguments.length)
      break;
  }
  
  return ret;
}

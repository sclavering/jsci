({
  /*
  object.keys(obj)
  
  Return an array of the names of the enumerable fields in |obj|.
  */
  keys: function(obj) {
    const ks = [];
    for(var key in obj) ks.push(key);
    return ks;
  },


  /*
  object.keys(obj)
  
  Return an array of values of the enumerable fields in |obj|.
  */
  values: function(obj) {
    const vs = [];
    for each(var val in obj) vs.push(val);
    return vs;
  },
})

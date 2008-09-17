({

  /**/

  _exists:

  function(key) {
    var res=cdb.cdb_make_exists(this.cdbmp, key, key.length);
    return res?true:false;
  },

  /**/

  _read:

  function (len, pos) {
    var buf=Pointer.malloc(len);
    var res=cdb.cdb_read(this.cdbp, buf, len, pos);
    if (res===0)
      return buf.string(len);
  },

/*

    obj.add(key,val)


Adds the key/value pair to the database. Both arguments should
be strings.

*/
  add: 

  function(key, val) {
    return cdb.cdb_make_add(this.cdbmp, key, key.length, val, val.length);
  },

/*

    obj.close()


Frees any memory allocated by the cdb library. If a file name was given to [[cdb]], the file is closed.
If a file object or file descriptor was given, the file remains open.

*/

  close:

  function() {
    if (this.cdbp!=undefined) this.free();
    if (this.cdbmp!=undefined) this.finish();
  },

/*

    obj.each()

Returns a key/value pair and advances iterator, unless it is
already at the end of the file.

### Return value ###

Returns [key, value] (an array of two strings)
or _undefined_ if there are no more keys in the database.

### See also ###

[[$curdir.reset]]

*/

  each:

  function() {
    if (!this.seqnext()) return;
    return [this.readkey(),this.readdata()];
  },

/*

    bool = obj.exists(key)

Returns _true_ or _false_ depending on whether the key exists in the database.
The key should be a string.

NOTE
----

This function can be used both in read and write mode, unlike
[[$curdir.find]], which can only be used in read mode.

*/

  exists:

  function(key) {
    if (this.cdbmp!=undefined) return this._exists(key);
    else return this.find(key);
  },


/*

    num = obj.fileno()

Returns the file number of the underlying file
*/

  fileno:

  function() {
    if (this.cdbp!=undefined) return this.cdbp.member(0,'cdb_fd').$;
    if (this.cdbmp!=undefined) return this.cdbmp.member(0,'cdb_fd').$;
  },

/*

    bool = obj.find(key)

Returns _true_ if a _key_ exists in the database, or _false_ otherwise.

NOTE
----

Can only be used in read mode. See also [[$curdir.exists]],
which also works in write mode.

*/

  find:
  
  function(key) {
    var res=cdb.cdb_find(this.cdbp, key, key.length);
    return res?true:false;
  },


/*

    bool = obj.findinit(key)

Returns _true_ if a _key_ exists in the database, or _false_ otherwise.
Used in conjunction with [[$curdir.findnext]] () to retrieve several
values with the same key.

NOTE
----

Can only be used in read mode.

*/

  findinit:

  function(key) {
    var res=cdb.cdb_findinit(this.cdbfp, this.cdbp, key, key.length);
    return res?true:false;
  },

/*

    bool = obj.findnext()

Returns _true_ if another record with the same key as found with
[[$curdir.findinit]]
exists, and advances the cursor to that key.

NOTE
----

Can only be used in read mode.

*/


  findnext:
  function() {
    var res=cdb.cdb_findnext(this.cdbfp);
    return res?true:false;
  },


  finish:

  function() {
    var ret=cdb.cdb_make_finish(this.cdbmp);
    switch(typeof(this.file)) {
    case "string": clib.close(this.fd); break;
    }
  },

  free:

  function() {
    cdb.cdb_free(this.cdbp);
    switch(typeof(this.file)) {
    case "string": clib.close(this.fd); break;
    }
  },

/*

    str = get(key, [unique=true])

Returns the value associated with _key_ or _undefined_
if the key does not exist.

If _unique_ is false,
returns an array of values associated with _key_ or
an empty array if _key_ does not exist.

NOTE
----

Can only be used in read mode.
*/

  get:

  function (key, unique) {
    if (unique===undefined)
      unique=true;
    
    if (unique) {
      if (this.find(key)) {
	return this.readdata();
      } else {
	return;
      }
    } else {
      var ret=[];
      if (this.findinit(key)) {
	do {
	  ret.push(this.readdata());
	} while(this.findnext());
      }
      return ret;
    }
  },
    
  init:

  function() {
    var fd;
    switch(typeof this.file) {
    case "string":
      if (JSEXT_config._WIN32)
	fd=this.fd=clib.open(this.file, clib.O_RDWR | clib.O_CREAT | clib.O_TRUNC | clib.O_BINARY, clib.S_IREAD | clib.S_IWRITE);
      else
	fd=this.fd=clib.open(this.file, clib.O_RDWR | clib.O_CREAT | clib.O_TRUNC, 0777);
      break;
    case "number":
      fd=this.file; break;
    case "object":
      fd=this.file.fileno(); break;
    }
    
    this.cdbp=Pointer(cdb['struct cdb']);
    this.cdbfp=Pointer(cdb['struct cdb_find']);
    
    var ret=cdb.cdb_init(this.cdbp, fd);
    if (ret==-1) {
      throw("Open Cdb");
    }
  },

/*

    obj.insert(key,val)

Inserts the key/value pair. Does not seem to work.

NOTE
----

Can only be used in write mode.
*/

  insert:
  
  function(key, val) {
    return this.put(key, val, $parent.PUT_INSERT);
  },

  put:

  function(key, val, flag) {
    var ret=cdb.cdb_make_put(this.cdbmp, key, key.length, val, val.length, flag);
    if (ret==-1)
      throw("cdb::put");
  },
    
/*


    obj2 = obj.read([unique=true])


Reads all values in database and returns an object with the same
key/value associations.

If unique is true (default), all values are strings.

If unique is false, all values are arrays of strings.
*/

  read:

  function(unique) {
    if (unique==undefined) unique=true;
    
    var ret={};
    
    if (unique==true) {
      this.reset();
      var pair;
      while ((pair=this.each())) {
	ret[pair[0]]=pair[1];
      }
    } else {
      this.reset();
      var pair;
      while ((pair=this.each())) {
	if (pair[0] in ret) ret[pair[0]].push(pair[1]);
	else ret[pair[0]]=[pair[1]];
      }
    }
    return ret;
  },

/*

    str = obj.readdata()

Returns a string containing the data in the current record

*/

  readdata:
  
  function() {
    return this._read(this.cdbp.member(0,'cdb_vlen').$, this.cdbp.member(0,'cdb_vpos').$);
  },

/*

    str = readkey()

Returns a string containing the key in the current record

*/

  readkey:

  function() {
    return this._read(this.cdbp.member(0,'cdb_klen').$, this.cdbp.member(0,'cdb_kpos').$);
  },

/*

    obj.replace(key,val)

Replaces the key/value pair. Does not seem to work.

*/

  replace:

  function(key, val) {
    return this.put(key, val, $parent.PUT_REPLACE);
  },

/*

    obj.reset()

Reset iterator. Use with [[$curdir.each]].

*/

  reset:

  function() {
    this.cptr=[2048];
  },

/*

    obj.seqinit()

Moves the cursor to the first record in the database.
Same as [[$curdir.reset]].
*/

  seqinit:

  function() {
    this.cptr=[2048];
  },

/*

    obj.seqnext()


Advances the cursor to the next record in the database.
Returns _true_ if the record exists, or _false_ if not.
*/

  seqnext:
  
  function() {
    if (this.cptr==undefined) this.seqinit();
    var res=cdb.cdb_seqnext(this.cptr,this.cdbp);
    return res?true:false;
  },

/*

    obj.set(key, value, [replace=true])

Inserts or replaces the key/value pair into the database. Replace does
not seem to work. Key and value should be strings.
*/

  set:

  function(key, value, replace) {
    if (replace===undefined)
      replace=false;
    if (replace) {
      return this.replace(key,value);
    } else {
      return this.add(key,value);
    }
  },

  start:

  function() {
    var fd;
    
    switch(typeof(this.file)) {
    case "string":
      if (JSEXT_config._WIN32)
	fd=this.fd=clib.open(this.file, clib.O_RDWR | clib.O_CREAT | clib.O_TRUNC | clib.O_BINARY, clib.S_IREAD | clib.S_IWRITE);
      else
	fd=this.fd=clib.open(this.file, clib.O_RDWR | clib.O_CREAT | clib.O_TRUNC, 0777);
      break;
    case "number":
      fd=this.file; break;
    case "object":
      fd=this.file.fileno(); break;
      
    }
    this.cdbmp=Pointer(cdb['struct cdb_make']);
    cdb.cdb_make_start(this.cdbmp,fd);
  },

  warn:

  function(key, val) {
    return this.put(key, val, $parent.PUT_WARN);
  },

/*

    obj.write(obj2)

Writes all properties of _obj2_ to database as key/value pairs. All properties should be strings.
*/

  write:

  function(obj) {
    var i;
    for (i in obj) {
      this.add(i,obj[i]);
    }
  }

})


/*

    new Cdb(file, [mode="r"])

Opens an existing cdb file or creates a new one. Based on
the tinycdb library. A cdb file consists key/value pairs.
A cdb file can be created once and not modified later.
Opening an existing cdb file in write mode erases all existing data.

### Arguments ###

* _file_: Can be a [[$curdir.File]] object, a filename (string) or
  a file descriptor (number)
* _mode_: 'r' or 'w'.

*/

(function(curdir){
  
  var Cdb=function(file, mode) {
    var obj;
    
    if (this.constructor != arguments.callee) {
      obj={};
      obj.__proto__=arguments.callee.prototype;
    } else
      obj=this;
    
    mode = mode || "r";
    
    obj.file=file;
    switch(mode) {
    case "r": 
      obj.init(); break;
    case "w":
      obj.start(); break;
    default:
      throw("Cdb: Wrong mode '"+mode+"'");
    }
    
    return obj;
  }

  Cdb.cdb=
    (function() {
      this['struct cdb']=Type.struct();
      this['struct cdb_find']=Type.struct();
      this['struct cdb_make']=Type.struct();
      this['struct cdb_rl']=Type.struct();
      this['dl 0'] = Dl("libcdb.so.1"); // $path + '/Cdb/libcdb.so';
      this['cdbi_t']=Type.unsigned_int;
      this['cdb_hash']=this['dl 0'].pointer('cdb_hash',Type['function'](Type.unsigned_int,[{'const':true,name:'buf',type:Type.pointer(Type['void'])},{'const':false,name:'len',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_unpack']=this['dl 0'].pointer('cdb_unpack',Type['function'](Type.unsigned_int,[{'const':true,name:'buf',type:Type.array(Type.unsigned_char,4)}],false,'cdecl')).$;
      this['cdb_pack']=this['dl 0'].pointer('cdb_pack',Type['function'](Type['void'],[{'const':false,name:'num',type:Type.unsigned_int},{'const':false,name:'buf',type:Type.array(Type.unsigned_char,4)}],false,'cdecl')).$;
      this['struct cdb']=(this['struct cdb'][0]={name:'cdb_fd',type:Type.int},this['struct cdb'][1]={name:'cdb_fsize',type:Type.unsigned_int},this['struct cdb'][2]={name:'cdb_dend',type:Type.unsigned_int},this['struct cdb'][3]={name:'cdb_mem',type:Type.pointer(Type.unsigned_char)},this['struct cdb'][4]={name:'cdb_vpos',type:Type.unsigned_int},this['struct cdb'][5]={name:'cdb_vlen',type:Type.unsigned_int},this['struct cdb'][6]={name:'cdb_kpos',type:Type.unsigned_int},this['struct cdb'][7]={name:'cdb_klen',type:Type.unsigned_int},this['struct cdb']);
      this['cdb_init']=this['dl 0'].pointer('cdb_init',Type['function'](Type.int,[{'const':false,name:'cdbp',type:Type.pointer(this['struct cdb'])},{'const':false,name:'fd',type:Type.int}],false,'cdecl')).$;
      this['cdb_free']=this['dl 0'].pointer('cdb_free',Type['function'](Type['void'],[{'const':false,name:'cdbp',type:Type.pointer(this['struct cdb'])}],false,'cdecl')).$;
      this['cdb_read']=this['dl 0'].pointer('cdb_read',Type['function'](Type.int,[{'const':true,name:'cdbp',type:Type.pointer(this['struct cdb'])},{'const':false,name:'buf',type:Type.pointer(Type['void'])},{'const':false,name:'len',type:Type.unsigned_int},{'const':false,name:'pos',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_get']=this['dl 0'].pointer('cdb_get',Type['function'](Type.pointer(Type['void']),[{'const':true,name:'cdbp',type:Type.pointer(this['struct cdb'])},{'const':false,name:'len',type:Type.unsigned_int},{'const':false,name:'pos',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_find']=this['dl 0'].pointer('cdb_find',Type['function'](Type.int,[{'const':false,name:'cdbp',type:Type.pointer(this['struct cdb'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int}],false,'cdecl')).$;
      this['struct cdb_find']=(this['struct cdb_find'][0]={name:'cdb_cdbp',type:Type.pointer(this['struct cdb'])},this['struct cdb_find'][1]={name:'cdb_hval',type:Type.unsigned_int},this['struct cdb_find'][2]={name:'cdb_htp',type:Type.pointer(Type.unsigned_char)},this['struct cdb_find'][3]={name:'cdb_htab',type:Type.pointer(Type.unsigned_char)},this['struct cdb_find'][4]={name:'cdb_htend',type:Type.pointer(Type.unsigned_char)},this['struct cdb_find'][5]={name:'cdb_httodo',type:Type.unsigned_int},this['struct cdb_find'][6]={name:'cdb_key',type:Type.pointer(Type['void'])},this['struct cdb_find'][7]={name:'cdb_klen',type:Type.unsigned_int},this['struct cdb_find']);
      this['cdb_findinit']=this['dl 0'].pointer('cdb_findinit',Type['function'](Type.int,[{'const':false,name:'cdbfp',type:Type.pointer(this['struct cdb_find'])},{'const':false,name:'cdbp',type:Type.pointer(this['struct cdb'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_findnext']=this['dl 0'].pointer('cdb_findnext',Type['function'](Type.int,[{'const':false,name:'cdbfp',type:Type.pointer(this['struct cdb_find'])}],false,'cdecl')).$;
      this['cdb_seqnext']=this['dl 0'].pointer('cdb_seqnext',Type['function'](Type.int,[{'const':false,name:'cptr',type:Type.pointer(Type.unsigned_int)},{'const':false,name:'cdbp',type:Type.pointer(this['struct cdb'])}],false,'cdecl')).$;
      this['cdb_seek']=this['dl 0'].pointer('cdb_seek',Type['function'](Type.int,[{'const':false,name:'fd',type:Type.int},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int},{'const':false,name:'dlenp',type:Type.pointer(Type.unsigned_int)}],false,'cdecl')).$;
      this['cdb_bread']=this['dl 0'].pointer('cdb_bread',Type['function'](Type.int,[{'const':false,name:'fd',type:Type.int},{'const':false,name:'buf',type:Type.pointer(Type['void'])},{'const':false,name:'len',type:Type.int}],false,'cdecl')).$;
      this['struct cdb_make']=(this['struct cdb_make'][0]={name:'cdb_fd',type:Type.int},this['struct cdb_make'][1]={name:'cdb_dpos',type:Type.unsigned_int},this['struct cdb_make'][2]={name:'cdb_rcnt',type:Type.unsigned_int},this['struct cdb_make'][3]={name:'cdb_buf',type:Type.array(Type.unsigned_char,4096)},this['struct cdb_make'][4]={name:'cdb_bpos',type:Type.pointer(Type.unsigned_char)},this['struct cdb_make'][5]={name:'cdb_rec',type:Type.array(Type.pointer(this['struct cdb_rl']),256)},this['struct cdb_make']);
      this['CDB_PUT_ADD']=0;
      this['CDB_FIND']=0;
      this['CDB_PUT_REPLACE']=1;
      this['CDB_FIND_REMOVE']=1;
      this['CDB_PUT_INSERT']=2;
      this['CDB_PUT_WARN']=3;
      this['CDB_PUT_REPLACE0']=4;
      this['CDB_FIND_FILL0']=4;
      this['cdb_make_start']=this['dl 0'].pointer('cdb_make_start',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])},{'const':false,name:'fd',type:Type.int}],false,'cdecl')).$;
      this['cdb_make_add']=this['dl 0'].pointer('cdb_make_add',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int},{'const':true,name:'val',type:Type.pointer(Type['void'])},{'const':false,name:'vlen',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_make_exists']=this['dl 0'].pointer('cdb_make_exists',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int}],false,'cdecl')).$;
      this['cdb_make_find']=this['dl 0'].pointer('cdb_make_find',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int},{'const':false,name:'mode',type:Type.int}],false,'cdecl')).$;
      this['cdb_make_put']=this['dl 0'].pointer('cdb_make_put',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])},{'const':true,name:'key',type:Type.pointer(Type['void'])},{'const':false,name:'klen',type:Type.unsigned_int},{'const':true,name:'val',type:Type.pointer(Type['void'])},{'const':false,name:'vlen',type:Type.unsigned_int},{'const':false,name:'mode',type:Type.int}],false,'cdecl')).$;
      this['cdb_make_finish']=this['dl 0'].pointer('cdb_make_finish',Type['function'](Type.int,[{'const':false,name:'cdbmp',type:Type.pointer(this['struct cdb_make'])}],false,'cdecl')).$;
      this['TINYCDB_VERSION']=0.76;
      this['cdb_datapos']=function (c) {var $a=arguments;with(this){with($a){return  ((c).member(0,'cdb_vpos').$) }}};
      this['cdb_datalen']=function (c) {var $a=arguments;with(this){with($a){return  ((c).member(0,'cdb_vlen').$) }}};
      this['cdb_keypos']=function (c) {var $a=arguments;with(this){with($a){return  ((c).member(0,'cdb_kpos').$) }}};
      this['cdb_keylen']=function (c) {var $a=arguments;with(this){with($a){return  ((c).member(0,'cdb_klen').$) }}};
      this['cdb_fileno']=function (c) {var $a=arguments;with(this){with($a){return  ((c).member(0,'cdb_fd').$) }}};
      this['cdb_readdata']=function (cdbp,buf) {var $a=arguments;with(this){with($a){return  cdb_read((cdbp), (buf), cdb_datalen(cdbp), cdb_datapos(cdbp)) }}};
      this['cdb_readkey']=function (cdbp,buf) {var $a=arguments;with(this){with($a){return  cdb_read((cdbp), (buf), cdb_keylen(cdbp), cdb_keypos(cdbp)) }}};
      this['cdb_getdata']=function (cdbp) {var $a=arguments;with(this){with($a){return  cdb_get((cdbp), cdb_datalen(cdbp), cdb_datapos(cdbp)) }}};
      this['cdb_getkey']=function (cdbp) {var $a=arguments;with(this){with($a){return  cdb_get((cdbp), cdb_keylen(cdbp), cdb_keypos(cdbp)) }}};
      this['cdb_seqinit']=function (cptr,cdbp) {var $a=arguments;with(this){with($a){return  ((*(cptr))=2048) }}};
      return this;
    }).call({});
  
  Cdb.PUT_ADD=Cdb.cdb.PUT_ADD;
  Cdb.PUT_INSERT=Cdb.cdb.PUT_INSERT;
  Cdb.PUT_REPLACE=Cdb.cdb.PUT_REPLACE;
  Cdb.PUT_WARN=Cdb.cdb.PUT_WARN;

  return Cdb;

})(this)

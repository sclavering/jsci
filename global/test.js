function(name, config, _dl, cwd) {
    Dl=_dl("./Dl.so");
    Type=_dl("./Type.so");
    Pointer=_dl("./Pointer.so");
dl=Dl();
puts=dl.pointer('puts',Type['function'](Type.int,[{'const':true,name:'__s',type:Type.pointer(Type.char)}],false,'cdecl')).$

    DIR=Type.struct();
    this['struct dirent']=Type.struct(),(this['struct dirent'][0]={name:'d_ino',type:Type.int},this['struct dirent'][1]={name:'d_off',type:Type.int},this['struct dirent'][2]={name:'d_reclen',type:Type.unsigned_short},this['struct dirent'][3]={name:'d_type',type:Type.unsigned_char},this['struct dirent'][4]={name:'d_name',type:Type.array(Type.char,256)});
    opendir=dl.pointer('opendir',Type['function'](Type.pointer(this['DIR']),[{'const':true,name:'__name',type:Type.pointer(Type.char)}],false,'cdecl')).$;
    readdir=dl.pointer('readdir',Type['function'](Type.pointer(this['struct dirent']),[{'const':false,name:'__dirp',type:Type.pointer(this['DIR'])}],false,'cdecl')).$;
    puts(String(this['struct dirent'].sizeof));
    x=opendir(".");
    puts("A");

    while (e=readdir(x)) {
	puts("A");
	puts(e.$.toSource());
    }
    //    puts("50");
}
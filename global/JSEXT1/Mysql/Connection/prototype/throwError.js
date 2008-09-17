// Used for reporting errors from the mysql api (private)

function() {
    throw new Error("mysql: "+$parent.$parent.$parent.decodeUTF8(lib.mysql_error(this.mysql).string()));
}

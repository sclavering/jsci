/*
Name

This class exists to allow substitution of table and column names when calling
_MysqlConnection.query()_.
*/
(function() {

function Name(str) {
  this.str = str;
}

Name.prototype = {
  toMysqlString: function() {
    return '`' + this.str + '`';
  },
}

return Name;

})()

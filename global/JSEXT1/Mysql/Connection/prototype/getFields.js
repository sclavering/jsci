/**/

function() {
  this.fieldNames=[];
  this.fieldTypes=[];
  this.fieldCharSet=[];
  
  var field;
  while ((field=lib.mysql_fetch_field(this.result))!=null) {
    this.fieldTypes.push(field.member(0,'type').$);
    this.fieldCharSet.push(field.member(0,'charsetnr').$);
    this.fieldNames.push(field.member(0,'name').$.string(field.member(0,'name_length').$));
  }

}

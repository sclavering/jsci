/*
      conn.seek(num)

  Moves the cursor backwards or forwards in the result set.

 */

function(number) {
  lib.mysql_data_seek(this.result, number);
  this.rowNumber=number;
}

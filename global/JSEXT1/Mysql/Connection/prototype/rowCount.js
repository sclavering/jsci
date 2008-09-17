/*
        num = conn.rowCount()

    Returns the number of rows in the result set.
*/

function() {
  if (this.result)
    return lib.mysql_num_rows(this.result);
}

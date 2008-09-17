/*
      new Sqlite( [filename] )

  An object representing an SQLite database.

  ### Arguments ###

  * _filename_: A string specifying the path to the file where the
    database is stored. If the file exists, the database in it is opened.
    Otherwise, a new file is created. If no filename is given, a new
    database is created in memory.

 */


function(filename) {
		filename = filename || ":memory:";

		var ptr = [null];
		var rc = Sqlite.lib.sqlite3_open(encodeUTF8(filename), ptr);

		this.db = ptr[0];
		this.db.finalize = Sqlite.lib.sqlite3_close;

		if (rc) {
				try {
						this.throwError();
				} finally {
						this.close();
				}
		}
}
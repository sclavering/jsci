/*
    db.close()

Close the database.

*/

function() {
		$parent.lib.sqlite3_close(this.db);
		this.db.finalize=null;
		delete this.db;
}

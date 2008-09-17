/*
  Represents an SQLite statement. These objects are created
  by the [[$curdir]] object.
 */

function(conn, stmt) {
		this.conn = conn;
		this.stmt = stmt;
		stmt.finalize=lib.sqlite3_finalize;
}
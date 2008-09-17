function() {
    throw new Error("Sqlite: "+$parent.lib.sqlite3_errmsg16(this.db).UCstring());
}
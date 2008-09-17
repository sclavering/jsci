/*

      sess = new Session(path, config)

  Class representing session storage. See separate article on [[sessions]].

 */

function(path, config) {
  config = config || {};
  if (config.db) {
    this.db = config.db;
  } else {
    this.db = new $parent.Sqlite(path+JSEXT_config.sep+"0-session.sqlite");
    if (this.db.query("select 0 from SQLITE_MASTER where type='table' and name='config'").length==0) {
      this.db.exec("create table session (timestamp integer, id text, value text, primary key (id))");
      this.db.exec("create index timestamp on session (timestamp)");
    }
  }

  this.idField=config.idField || "JSX_sessionid";
  this.linger=config.linger || 3600;
  this.path=config.path || null;
  this.table=config.table || "session";
}

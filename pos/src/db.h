#ifndef DB_H_
#define DB_H_

PGconn* db_connect(DBConfig* db_config);
void db_disconnect(PGconn* connection);

#endif /* DB_H_ */

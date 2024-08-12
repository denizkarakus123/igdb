#ifndef DB_H
#define DB_H
#include <time.h>

#define HANDLE_MAX_LENGTH 32
#define COMMENT_MAX_LENGTH 64
typedef struct Record {
	char handle[HANDLE_MAX_LENGTH];
	unsigned long follower_count;
	char comment[COMMENT_MAX_LENGTH];
	time_t date_last_modified;

} Record;

typedef struct Database {
	Record *records;
	int size;
	int capacity;

} Database;

Database db_create();

void db_append(Database * db, Record const * item);

Record * db_index(Database * db, int index);

Record * db_lookup(Database * db, char const * handle);

void db_free(Database * db);

void db_load_csv(Database * db, char const * path);

void db_write_csv(Database * db, char const * path);

#endif

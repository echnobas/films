#include <sqlite3.h>
#include <stdio.h>

sqlite3 *open_database(const char * const path);
char *init_tables(sqlite3 * db);

#ifndef LOG_ENABLED
#define LOG_ENABLED 0
#endif

#define debug_print(...)                  \
	do                                    \
	{                                     \
		if (LOG_ENABLED)                  \
			fprintf(stderr, __VA_ARGS__); \
	} while (0)

int main()
{
    sqlite3* db = open_database("database.db");
    {
        char * err_msg = init_tables(db);
        if (err_msg != NULL)
        {
            debug_print("WARN! SQL error while initializing tables: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
    printf("Success\n");
    sqlite3_close(db);
}

sqlite3 * open_database(const char * const path)
{
	sqlite3 *db;
	int rc;
	rc = sqlite3_open(path, &db);
	if (rc != SQLITE_OK)
	{
		debug_print("ERROR! Can't open database: %s\n", sqlite3_errmsg(db));
		return NULL;
	}
	debug_print("LOG! Opened database successfully!\n");
	return db;
}

char * init_tables(sqlite3 * db)
{
	char *sql;
	char *err_msg = NULL;
	sql = "CREATE TABLE users("
		  "id 	  INTEGER   PRIMARY KEY,"
		  "username TEXT	NOT NULL,"
		  "password TEXT	NOT NULL);";
	sqlite3_exec(db, sql, NULL, 0, &err_msg); // Ignore status code, we don't really need it
    return err_msg;
}
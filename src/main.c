#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sysexits.h>

sqlite3 *open_database(const char * const path);
char *init_tables(sqlite3 * db);
char * read_f(FILE * f);
static void sigint(int _);
static const char* HELP = "whatever"
"h";

#ifndef LOG_ENABLED
#define LOG_ENABLED 0
#endif
#define UNIMPLEMENTED 1;

#define debug_print(...)                  \
	do                                    \
	{                                     \
		if (LOG_ENABLED)                  \
			fprintf(stderr, __VA_ARGS__); \
	} while (0)

int main()
{
	int status_code = 0;
	signal(SIGINT, sigint);
    sqlite3* db = open_database("database.db");
    {
        char * err_msg = init_tables(db);
        if (err_msg != NULL)
        {
            debug_print("WARN! SQL error while initializing tables: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
	while (1)
	{
		printf("> ");
		/* Write entire buffer to screen immediately */
		fflush(stdout); 
		char * command = read_f(stdin);
		debug_print("RAW: %s; LEN: %d;\n", command, strlen(command));
		if (strcmp(command, "quit") == 0)
			break;
		else if (strcmp(command, "new") == 0)
			status_code = EX_USAGE; /* TODO: new_user; unimplemented - normally something like `status_code = new_user(...)` */
		else
			status_code = EX_USAGE;
		free(command);
	}
    sqlite3_close(db);
	return status_code;
}

static void sigint(int _)
{
	/* I have no idea why this is here but i'm too afraid to remove it */
    (void)_;
	/*
		\b is backspace \r is carriage return, here we are *hoping* that we can erase the ^C
		TODO: find better solution - termios?
	*/
	printf("\b\b\r!!! <Ctrl-C> - Use `quit` to quit the program!\n> ");
	fflush(stdout);
}

char * read_f(FILE * f)
{
	size_t n = 0;
	int result;
	char *buffer;
	result = getline(&buffer, &n, f);
	if (result < 0)
		return NULL;
	/* Strip newline */
	if (buffer[result - 1] == '\n') {
    	buffer[result - 1] = '\0';
	}
	return buffer;
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
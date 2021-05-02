#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sysexits.h>
#include <assert.h>
#include <openssl/sha.h>

#include "user.h"
#define TRUE 1
#define FALSE 0

sqlite3 *open_database(const char * const path);
char * init_tables(sqlite3 * db);
char * read_f(FILE * f);
struct User * login_user(sqlite3 * db, char * username, char * password);
unsigned char * hash_password(char * input);
static void sigint(int _);
int new_user();

static const char * HELP = "whatever"
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
		{
			free(command);
			break;
		}
		else if (strcmp(command, "new") == 0)
			status_code = new_user(db);
		else if (strcmp(command, "login") == 0)
		{
			printf("Enter username\n> ");
			char * username = read_f(stdin);
			printf("Enter password\n> ");
			char * password = read_f(stdin);
			if (username == NULL || password == NULL)
			{
				if (password != NULL)
					free(password);
				if (username != NULL)
					free(username);
				free(command);
				break;
			}
			struct User * user = login_user(db, username, password);
			if (user != NULL)
			{
				printf("Success!!\n");
				free(user);
			}
			free(username);
			free(password);
			fflush(stdout);
		}
		else
			status_code = EX_USAGE;
		free(command);
	}
    sqlite3_close(db);
	return status_code;
}


unsigned char * hash_password(char * input)
{
	int length = strlen(input);
	SHA256_CTX ctx;
	unsigned char * out = malloc(sizeof(unsigned char) * SHA256_DIGEST_LENGTH);
	if (!out)
		return NULL;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, (unsigned char *)input, length);
	SHA256_Final(out, &ctx);
	return out;
}


struct User * login_user(sqlite3 * db, char * username, char * password)
{
	sqlite3_stmt * stmt;
	struct User * user = malloc(sizeof(struct User));
	if (user == NULL)
		return NULL;
	strcpy(user->username, "\0\0");
	memset(user->password, 0, 32);

	if (sqlite3_prepare_v3(db, "SELECT * FROM users WHERE username = ?;", -1, 0, &stmt, NULL) != SQLITE_OK)
		return NULL;
	sqlite3_bind_text(stmt, 1, username, -1, NULL);
	for(;;)
	{
		int rc = sqlite3_step(stmt);
		if (rc == SQLITE_DONE)
			break;
		if (rc != SQLITE_ROW)
			return NULL;
		memcpy(user->username, sqlite3_column_text(stmt, 1), 20);
		memcpy(user->password, sqlite3_column_blob(stmt, 2), 32);
	}
	sqlite3_finalize(stmt);
	if (strcmp(user->username, "\0\0") == 0)
	{
		free(user);
		return NULL;
	}
	unsigned char * password_hash = hash_password(password);
	if (password_hash == NULL)
	{
		free(user);
		return NULL;
	}
	if (memcmp(password_hash, user->password, 32) == 0)
	{
		free(password_hash);
		return user;
	}
	free(password_hash);
	free(user);
	return NULL;
}

int new_user(sqlite3 * db)
{
	/* Theoretically these pointers should always be NULL or point to valid memory [read_f], but I wouldn't put it past C */
	char * username = NULL;
	char * password = NULL;
	char * password_hash = NULL;
	struct User user;

	printf("Enter username\n> ");
	fflush(stdout);
	username = read_f(stdin);

	printf("Enter password for new account `%s`\n> ", username);
	fflush(stdout);
	password = read_f(stdin);

	if (!username || !password)
		return 1;

	password_hash = hash_password(password);
	if (!password_hash)
		return 1;

	/* Can't wait to debug some null pointers! woohoo! */
	strncpy(user.username, username, 20);
	strncpy(user.password, password_hash, SHA256_DIGEST_LENGTH);
	insert_user(db, &user);
	debug_print("TRACE! Username: %s\n", user.username);
	debug_print("TRACE! Password: ");
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		debug_print("%0x", user.password[i]);
	debug_print("\n");
	/* I really hope this doesn't invalidate something later... */
	free(username);
	free(password);
	free(password_hash);
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

/*
	26/04/21
	---------------------------
	for some reason valgrind doesn't like this function
	FIXMEs
	> ==6310== Conditional jump or move depends on uninitialised value(s)
	==6310==    at 0x4A26DE1: getdelim (in /usr/lib/libc-2.33.so)
	==6310==    by 0x1094ED: read_f (main.c:107)
	==6310==    by 0x1092D9: main (main.c:49)
	==6310==
	---------------------------
	27/04/21
	Dear future me, please don't leave pointers uninitialized as they will point to arbitrary memory
*/
char * read_f(FILE * f)
{
	size_t n = 0;
	int result = 0;
	char * buffer = NULL;
	result = getline(&buffer, &n, f);
	// If *lineptr is set to NULL and *n is set 0 before the call, then
    //    getline() will allocate a buffer for storing the line.  This
    //    buffer should be freed by the user program even if getline()
    //    failed.
	if (result < 0)
	{
		free(buffer);
		return NULL;
	}
	/* Strip newline */
	if (buffer[result - 1] == '\n') {
    	buffer[result - 1] = '\0';
	}
	return buffer;
}

sqlite3 * open_database(const char * const path)
{
	sqlite3 * db = NULL;
	int rc = 0;
	rc = sqlite3_open(path, &db);
	if (rc != SQLITE_OK)
	{
		debug_print("ERROR! Can't open database: %s\n", sqlite3_errmsg(db));
		return NULL;
	}
	assert(db != NULL);
	debug_print("LOG! Opened database successfully!\n");
	return db;
}

char * init_tables(sqlite3 * db)
{
	char * sql = NULL;
	char * err_msg = NULL;
	sql = "CREATE TABLE users("
		  "id 	  INTEGER   PRIMARY KEY,"
		  "username TEXT	NOT NULL,"
		  "password BLOB	NOT NULL);";
	sqlite3_exec(db, sql, NULL, 0, &err_msg); // Ignore status code, we don't really need it
    return err_msg;
}
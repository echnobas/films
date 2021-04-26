#include <sqlite3.h>

struct User {
	unsigned int id;
	char username[20];
	char password[20];
};

char* insert_user(sqlite3 * db, struct User * user);
void print_users(sqlite3 * db);
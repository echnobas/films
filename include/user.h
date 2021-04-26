#include <sqlite3.h>
#include <openssl/sha.h>

struct User {
	unsigned int id;
	char username[20];
	unsigned char password[SHA256_DIGEST_LENGTH];
};

char* insert_user(sqlite3 * db, struct User * user);
void print_users(sqlite3 * db);
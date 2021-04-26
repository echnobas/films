#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"

char *insert_user(sqlite3 * db, struct User * user)
{
	char sql[400];
	char *err_msg = NULL;
	snprintf(sql, 399,
			 "INSERT INTO users(username, password) VALUES("
			 "'%s', '%s');",
			 user->username, user->password);
	sqlite3_exec(db, sql, NULL, 0, &err_msg);
	return err_msg;
}
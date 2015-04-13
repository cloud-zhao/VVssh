#include <sqlite3.h>
#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _STRING_h
#include <string.h>
#endif

typedef struct Result
{
	const char *hostname[10];
	const char *ip[10];
	const char *user[10];
	const char *password[10];
	const char *role[10];
}Res;

sqlite3 *db;
char *errmsg;
int ret;
sqlite3_stmt *stmt;

// insert hostinfo
int sqlite3_insert(char *host,char *ip,char *user,char *pass,char *role);

// select hostinfo
int sqlite3_select(char *whereid,char *wherevlaue,Res *res);

// connect database;
int _sqlite3_connect(void);

// create tables;
int sqlite3_create(void);

// close database link
int _sqlite3_disconnect(void);

//get_all_table to char ***pazResult
int sqlite3_table(char *whereid,char *wherevlaue,char *result,int *count);
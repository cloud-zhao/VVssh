#include <sqlite3.h>
#ifndef _STDIO_H
#include <stdio.h>
#endif
#ifndef _STRING_h
#include <string.h>
#endif
#ifndef _STDLIB_h
#include <stdlib.h>
#endif
#ifndef _MALLOC_h
#include <malloc.h>
#endif

typedef struct Result
{
	char *hostname[10];
	char *ip[10];
	char *user[10];
	char *password[10];
	char *role[10];
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
int sqlite3_alltable(char *whereid,char *wherevlaue,char **result,int *count);

//initialization struct result;
Res* init_res(void);
char** init_Res(void);

//free struct result;
int free_res(Res *prr);
int free_Res(char **pres);

#ifndef _VVSQLITE_H
#define _VVSQLITE_H

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

typedef struct Result
{
	char *hostname[10];
	char *ip[10];
	char *user[10];
	char *password[10];
	char *pkey[10];
	char *role[10];
}Res;

/*
sqlite3 *db;
char *errmsg;
int ret;
sqlite3_stmt *stmt;
*/

// insert hostinfo
int sqlite3_insert(char *host,char *ip,char *user,char *pass,char *pkey,char *role);

//delete hostinfo
int sqlite3_delete(char *host,char *ip);

// select hostinfo
int sqlite3_select(char *whereid,char *wherevlaue,Res *res);

// connect database;
//static int _sqlite3_connect(void);

// create tables;
int sqlite3_create(void);

// close database link
//static int _sqlite3_disconnect(void);

//get_all_table to char ***pazResult
int sqlite3_alltable(char *whereid,char *wherevlaue,char **result,int *count);

//check host or ip exists
int sqlite3_checkinfo(const char *id,const char *value);

//absolute path
int absolute_path(char *pwd);

//initialization struct result;
Res* init_res(void);
char** init_Res(void);

//free struct result;
int free_res(Res *prr);
int free_Res(char **pres);

#endif

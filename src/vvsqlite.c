#include "vvsqlite.h"

int _sqlite3_disconnect(void){
	if(db==NULL){
		return 0;
	}
	sqlite3_close(db);
	return 0;
}

int _sqlite3_connect(void){

	ret=sqlite3_open("./../data/mydata.db",&db);
	if(ret != SQLITE_OK){
		fprintf(stderr,"Cannot open db:%s\n",sqlite3_errmsg(db));
		return 1;
	}

	printf("Open database\n");
	return 0;
}

int sqlite3_create(void){
	char *sql_create_table="create table hostinfo(hostname varchar(50) not null,ip varchar(20) not null primary key,user varchar(20) default 'root',password varchar(255) not null,role varchar(255))";

	if(_sqlite3_connect())
		return 1;
        ret=sqlite3_exec(db,sql_create_table,NULL,NULL,&errmsg);
        if(ret != SQLITE_OK){
                fprintf(stderr,"Create table fail:%s\n",errmsg);
		sqlite3_free(errmsg);
		_sqlite3_disconnect();
		return 1;
        }
        sqlite3_free(errmsg);
	_sqlite3_disconnect();
	return 0;
}

int sqlite3_insert(char *host,char *ip,char *user,char *pass,char *role){
	char *sql_insert="insert into hostinfo values(?,?,?,?,?)";
	if(user==NULL)
		user="root";
	if(role==NULL)
		role="default";

	if(_sqlite3_connect())
		return 1;
        ret=sqlite3_prepare_v2(db,sql_insert,-1,&stmt,0);
        if(ret != SQLITE_OK){
                fprintf(stderr,"Sql prepare fail\n");
		sqlite3_finalize(stmt);
		_sqlite3_disconnect();
		return 1;
        }else{
                sqlite3_bind_text(stmt,1,host,strlen(host),NULL);
                sqlite3_bind_text(stmt,2,ip,strlen(ip),NULL);
                sqlite3_bind_text(stmt,3,user,strlen(user),NULL);
                sqlite3_bind_text(stmt,4,pass,strlen(pass),NULL);
                sqlite3_bind_text(stmt,5,role,strlen(role),NULL);
                sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);

	return 0;
}

int sqlite3_select(char *whereid,char *wherevlaue,Res *res){
	char *sql_select="select * from hostinfo where hostname=?";

	if(_sqlite3_connect())
		return 1;
	ret=sqlite3_prepare_v2(db,sql_select,-1,&stmt,0);
        if(ret != SQLITE_OK){
                fprintf(stderr,"Sql prepare fail");
		sqlite3_finalize(stmt);
		_sqlite3_disconnect();
		return 1;
        }else{
                //sqlite3_bind_text(stmt,1,whereid,strlen(whereid),NULL);
                sqlite3_bind_text(stmt,1,wherevlaue,strlen(wherevlaue),NULL);
                int row=0;
                while(1){
                        ret=sqlite3_step(stmt);
                        if(ret == SQLITE_ROW){
				int ncol=sqlite3_column_count(stmt);
				int i;
				for(i=0;i<ncol;i++){
					res->hostname[row]=sqlite3_column_text(stmt,0);
					res->ip[row]=sqlite3_column_text(stmt,1);
					res->user[row]=sqlite3_column_text(stmt,2);
					res->password[row]=sqlite3_column_text(stmt,3);
					res->role[row]=sqlite3_column_text(stmt,4);
				}
                                row++;
                        }else if(ret == SQLITE_DONE){
                                break;
                        }else{
                                fprintf(stderr,"Select Failed.\n");
				sqlite3_finalize(stmt);
				_sqlite3_disconnect();
                                return 1;
                        }
                }
        }
        sqlite3_finalize(stmt);

	return 0;
}

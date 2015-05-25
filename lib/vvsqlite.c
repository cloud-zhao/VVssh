#include "vvsqlite.h"

static sqlite3 *db;
static char *errmsg;
static int ret;
static sqlite3_stmt *stmt;

static int _sqlite3_disconnect(void){
	if(db==NULL){
		return 0;
	}
	sqlite3_close(db);
	return 0;
}

static int _sqlite3_connect(void){
	char datafile[1024]="./../data/";
	struct passwd *pwd;

	pwd=getpwuid(getuid());
	if(pwd==NULL){
		fprintf(stderr,"Get user error.\n");
		return 1;
	}else{
		strcat(datafile,pwd->pw_name);
		strcat(datafile,"_data.db");
	}

	ret=sqlite3_open(datafile,&db);
	if(ret != SQLITE_OK){
		fprintf(stderr,"Cannot open db:%s\n",sqlite3_errmsg(db));
		return 1;
	}

	//printf("Open database\n");
	return 0;
}

int sqlite3_create(void){
	char *sql_create_table="create table hostinfo(hostname varchar(50) not null,ip varchar(20) not null,user varchar(20) default 'root',password varchar(255),pkey varchar(100),role varchar(255),primary key (hostname,ip))";

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
	//printf("Database install complete.\n");
	return 0;
}

int sqlite3_insert(char *host,char *ip,char *user,char *pass,char *pkey,char *role){
	char *sql_insert="insert into hostinfo values(?,?,?,?,?,?)";
	
	if((host==NULL) || (ip==NULL) || (user==NULL) || (pass==NULL) || (pkey==NULL) || (role==NULL)){
		fprintf(stderr,"Parameter error:must provide host,ip,user,pass,key,role.\n");
		return 1;
	}

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
                sqlite3_bind_text(stmt,5,pkey,strlen(pkey),NULL);
                sqlite3_bind_text(stmt,6,role,strlen(role),NULL);
                sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);

	return 0;
}

int sqlite3_delete(char *host,char *ip){
	char sql_delete[100];

	if((host==NULL)&&(ip==NULL)){
		fprintf(stderr,"Must set host or ip.\n");
		return 1;
	}
	if(host!=NULL){
		strcpy(sql_delete,"delete from hostinfo where hostname=");
		strcat(sql_delete,"'");
		strcat(sql_delete,host);
		strcat(sql_delete,"';");
	}else if(ip!=NULL){
		strcpy(sql_delete,"delete from hostinfo where ip=");
		strcat(sql_delete,"'");
		strcat(sql_delete,ip);
		strcat(sql_delete,"';");
	}

	if(_sqlite3_connect())
		return 1;
        ret=sqlite3_exec(db,sql_delete,NULL,NULL,&errmsg);
        if(ret != SQLITE_OK){
                fprintf(stderr,"delete row failed:%s\n",errmsg);
		sqlite3_free(errmsg);
		_sqlite3_disconnect();
		return 1;
        }
        sqlite3_free(errmsg);
	_sqlite3_disconnect();

	return 0;
}

int sqlite3_select(char *whereid,char *wherevlaue,Res *res){
	char sql_selects[255]="select * from hostinfo where ";
	char *sql_select=sql_selects;
	strcat(sql_select,whereid);
	strcat(sql_select,"=?");

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
				strcpy(res->hostname[row],sqlite3_column_text(stmt,0));
				strcpy(res->ip[row],sqlite3_column_text(stmt,1));
				strcpy(res->user[row],sqlite3_column_text(stmt,2));
				strcpy(res->password[row],sqlite3_column_text(stmt,3));
				strcpy(res->pkey[row],sqlite3_column_text(stmt,4));
				strcpy(res->role[row],sqlite3_column_text(stmt,5));
				row++;
                        }else if(ret == SQLITE_DONE){
				fprintf(stderr,"Already select done\n");
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

int sqlite3_alltable(char *whereid,char *wherevlaue,char **result,int *count){
	char sql_select[255]="select * from hostinfo";
	if((whereid==NULL)&&(wherevlaue==NULL)){
		strcat(sql_select,";");
	}else{
		strcat(sql_select," where ");
		strcat(sql_select,whereid);
		strcat(sql_select,"='");
		strcat(sql_select,wherevlaue);
		strcat(sql_select,"';");
	}
	char **presult;
	int nrow,ncol,i,j;

	if(_sqlite3_connect())
			return 1;
	ret=sqlite3_get_table(db,sql_select,&presult,&nrow,&ncol,&errmsg);
	if(errmsg!=NULL){
		fprintf(stderr,"get table fail:%s\n",errmsg);
		sqlite3_free_table(presult);
		sqlite3_free(errmsg);
		_sqlite3_disconnect();
		return 1;
	}
	if(ret==SQLITE_OK){
		if(nrow==0){
			sqlite3_free_table(presult);
			sqlite3_free(errmsg);
			_sqlite3_disconnect();
			return -1;
		}
		int nn=ncol;
		for(j=0;j<nrow;j++)
			for(i=0;i<ncol;i++)
				strcpy(result[nn-ncol],presult[nn++]);
		
		*count=nrow;
	}
	sqlite3_free_table(presult);
	sqlite3_free(errmsg);
	_sqlite3_disconnect();
	return 0;
}

int _mycb(void *fg,int ncol,char **resultcol,char **namecol){
	int *flag_ret=(int*)fg;
	*flag_ret=0;
	return 1;
}

int sqlite3_checkinfo(const char *id,const char *value){
	char sql_select[255]="select * from hostinfo where ";

	if((value==NULL)||(id==NULL))
		return 1;

	strcat(sql_select,id);
	strcat(sql_select,"='");
	strcat(sql_select,value);
	strcat(sql_select,"';");

	int flag_ret=2;

	if(_sqlite3_connect())
			return 1;
	ret=sqlite3_exec(db,sql_select,&_mycb,&flag_ret,&errmsg);
	if((ret!=SQLITE_ABORT)&&(ret!=SQLITE_OK)){
		fprintf(stderr,"Sql exec failed:%s\n",errmsg);
		sqlite3_free(errmsg);
		_sqlite3_disconnect();
		return 1;
	}

	sqlite3_free(errmsg);
	_sqlite3_disconnect();
	return flag_ret;
}

Res* init_res(void){
        Res *prr=(Res*)malloc(sizeof(Res));
        int i;
        for(i=0;i<10;i++)
                prr->hostname[i]=(char*)malloc(sizeof(char)*100);
        for(i=0;i<10;i++)
                prr->ip[i]=(char*)malloc(sizeof(char)*100);
        for(i=0;i<10;i++)
                prr->user[i]=(char*)malloc(sizeof(char)*100);
        for(i=0;i<10;i++)
                prr->password[i]=(char*)malloc(sizeof(char)*100);
        for(i=0;i<10;i++)
                prr->pkey[i]=(char*)malloc(sizeof(char)*100);
        for(i=0;i<10;i++)
                prr->role[i]=(char*)malloc(sizeof(char)*100);
        return prr;
}

int free_res(Res *prr){
	int i;
	for(i=0;i<10;i++)
		free(prr->hostname[i]);
	for(i=0;i<10;i++)
		free(prr->ip[i]);
	for(i=0;i<10;i++)
		free(prr->user[i]);
	for(i=0;i<10;i++)
		free(prr->password[i]);
	for(i=0;i<10;i++)
		free(prr->pkey[i]);
	for(i=0;i<10;i++)
		free(prr->role[i]);
	free(prr);
	return 0;
}

char** init_Res(void){
	char **pres;
	int i=0;
	pres=(char**)malloc(sizeof(char*)*100);
	for(;i<100;i++)
		pres[i]=(char*)malloc(sizeof(char)*100);
	return pres;
}

int free_Res(char **pres){
	int i=0;
	for(;i<100;i++)
		free(pres[i]);
	return 0;
}

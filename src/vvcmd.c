#include "../lib/vvsqlite.h"
#include "../lib/vvlibssh.h"



static int ifstr(char *ps,char pt);
static int ifip(const char *ip);
static int _help(void);

int main(int argc,char *argv[]){

	int rc;
	char *hostname=NULL;
	char *user=NULL;
	char *pass=NULL;
	char *ip=NULL;
	char *key=NULL;
	char *role=NULL;
	char *all=NULL;
	char *cmd=NULL;

	if((argc!=2)||(argc!=3)){
		_help();
		return 1;
	}

	if(argc==3){
		if(!ifip(argv[1])){
			rc=sqlite3_checkinfo("ip",argv[1]);
			if(rc==0){
				ip=argv[1];
			}else if(rc==2){
				fprintf(stderr,"Error:unknown ip address %s.",host);
				return 1;
			}
		}else if(!strcmp(argv[1],"all"))
			all=argv[1];
		else{
			rc=sqlite3_checkinfo("hostname",argv[1]);
			if(rc==2){
				rc=sqlite3_checkinfo("role",argv[1]);
				if(rc==2){
					fprintf(stderr,"Error:unknown host or role %s.",argv[1]);
					return 1;
				}else if(rc==0){
					role=argv[1];
				}
			}else if(rc==0){
				hostname=argv[1];
			}
		}
		if(rc==1){
			fprintf(stderr,"Error:database query failed.\n");
			return 1;
		}

		cmd=argv[2];
		if(ip!=NULL){
			sqlite3_alltable("ip",ip,sql_result,&sql_count);

		}else if(all!=NULL){
	
		}else if(role!=NULL){
	
		}else if(hostname!=NULL){

		}
	}else if(argc==2){
		
	}


	return 0;
}


static int ifip(const char *ip){
	unsigned long addr;
	if(ip==NULL)
		return 1;
	addr=inet_addr(ip);
	if(addr==INADDR_NONE)
		return 1;
	else
		return 0;
}

static int _help(void){
	printf("Usage:\n\t");
	printf("./vvcmd host[|ip|role|all] command \n\t");
	printf("cat file | ./vvcmd command\n");
	printf("Description:\n\t");
	printf("According to the host and IP executive order\n\t");
	printf("Press standard input batch execution commands\n");
	printf("STDIN FORMAT:\n\t");
	printf("host[|ip]\t[user]\t[password] [key]\n\n");
	printf("For more see source code.\n");
	return 0;
}


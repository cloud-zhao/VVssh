#include "../lib/vvsqlite.h"
#include "../lib/vvlibssh.h"



static int ifstr(char *ps,char pt);
static int ifip(const char *ip);
static int ip_cmd(void);
static int host_cmd(void);
static int free_host(void);
static int _help(void);

static char *hostname=NULL;
static char *user=NULL;
static char *pass=NULL;
static char *ip=NULL;
static char *key=NULL;
static char *role=NULL;
static char *all=NULL;
static char *cmd=NULL;

int main(int argc,char *argv[]){

	int rc;

	if((argc!=2)&&(argc!=3)){
		_help();
		return 1;
	}

	if(argc==3){
		if(!ifip(argv[1])){
			rc=sqlite3_checkinfo("ip",argv[1]);
			if(rc==0){
				ip=argv[1];
			}else if(rc==2){
				fprintf(stderr,"Error:unknown ip address %s.",argv[1]);
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
			ip_cmd();
		}else if(all!=NULL){
			char **sql_result=init_Res();
			int sql_count,i;

			sqlite3_alltable(NULL,NULL,sql_result,&sql_count);

			for(i=0;i<sql_count;i++){
				hostname=sql_result[i];
				ip=sql_result[++i];
				user=sql_result[++i];
				pass=sql_result[++i];
				key=sql_result[++i];
				role=sql_result[++i];
				printf("%s\t%s\t%s\t",hostname,ip,role);
				if(ssh_cmd(ip,user,pass,key,cmd,NULL))
					printf("ERROR\n");
			}

			free_Res(sql_result);
		}else if(role!=NULL){
			char **sql_result=init_Res();
			int sql_count,i;

			sqlite3_alltable("role",role,sql_result,&sql_count);

			for(i=0;i<sql_count;i++){
				hostname=sql_result[i];
				ip=sql_result[++i];
				user=sql_result[++i];
				pass=sql_result[++i];
				key=sql_result[++i];
				++i;
				printf("%s\t%s\t%s\t",hostname,ip,role);
				if(ssh_cmd(ip,user,pass,key,cmd,NULL))
					printf("ERROR\n");
			}

			free_Res(sql_result);
		}else if(hostname!=NULL){
			host_cmd();
		}
	}else if(argc==2){
		if(!strcmp(argv[1],"-H")){
			_help();
			return 0;
		}else{
			char content[256];
			cmd=argv[1];

			while(fgets(content,256,stdin)!=NULL){
				int len=strlen(content);
				if(content[len-1]=='\n')
					content[len-1]='\0';
				if(!ifip(content)){
					rc=sqlite3_checkinfo("ip",content);
					if(rc==0){
						ip=content;
					}else if(rc==2){
						fprintf(stderr,"Error:unknown ip address %s.",content);
						continue;
					}
				}else{
					rc=sqlite3_checkinfo("hostname",content);
					if(rc==2){
						fprintf(stderr,"Error:unknown host name%s.",content);
						continue;
					}else if(rc==0){
						hostname=content;
					}
				}
				if(rc==1){
					fprintf(stderr,"Error:database query failed.\n");
					return 1;
				}
				if(ip!=NULL)
					ip_cmd();
				else if(hostname!=NULL)
					host_cmd();
			}
		}
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

static int ip_cmd(void){
	char **sql_result=init_Res();
	int sql_count;

	sqlite3_alltable("ip",ip,sql_result,&sql_count);
	hostname=sql_result[0];
	user=sql_result[2];
	pass=sql_result[3];
	key=sql_result[4];
	role=sql_result[5];
	printf("%s\t%s\t%s\t",hostname,ip,role);
	if(ssh_cmd(ip,user,pass,key,cmd,NULL))
		printf("ERROR\n");

	free_Res(sql_result);
	sql_count=0;
	free_host();
	return 0;
}

static int host_cmd(void){
	char **sql_result=init_Res();
	int sql_count;

	sqlite3_alltable("hostname",hostname,sql_result,&sql_count);
	ip=sql_result[1];
	user=sql_result[2];
	pass=sql_result[3];
	key=sql_result[4];
	role=sql_result[5];
	printf("%s\t%s\t%s\t",hostname,ip,role);
	if(ssh_cmd(ip,user,pass,key,cmd,NULL))
		printf("ERROR\n");
	
	free_Res(sql_result);
	sql_count=0;
	free_host();
	return 0;
}

static int free_host(void){	
	hostname=NULL;
	user=NULL;
	pass=NULL;
	ip=NULL;
	key=NULL;
	role=NULL;
	all=NULL;
	return 0;
}

static int _help(void){
	printf("Usage:\n\t");
	printf("./vvcmd host[|ip|role|all] command \n\t");
	printf("cat file | ./vvcmd command\n");
	printf("Description:\n\t");
	printf("According to the host or IP or Role or ALL executive order\n\t");
	printf("Press standard input batch execution commands\n");
	printf("STDIN FORMAT:\n\t");
	printf("host[|ip]\t[user]\t[password] [key]\n\n");
	printf("Use -H option printf help info.\n");
	printf("For more see source code.\n");
	return 0;
}

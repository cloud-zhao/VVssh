#include "vvsqlite.h"
#include "vvlibssh.h"
#include "vvhead.h"



static int ifstr(char *ps,char pt);
static int formatpara(char *para);
static int strpara(char *para,char *pd[],char pp[]);
static int ifip(const char *ip);
static int ifdir(const char *path);
static int ptype(char *path,int flag);
static int _help(void);

int main(int argc,char *argv[]){
	int recv_send;
	char *host=NULL;
	char *hostname=NULL;
	char *user=NULL;
	char *pass=NULL;
	char *ip=NULL;
	char *key=NULL;
	char *role=NULL;
	char *all=NULL;
	char *local_path=NULL;
	char *remote_path=NULL;
	struct stat fileinfo;
	FILE *local_file;
	int rc=0;
	char all_para[1024];
	char *pa[3];

	//Parameter analysis
	if((argc!=3)&&(argc!=4)&&((argc==2)&&(!strcmp(argv[1],"-H")))){
		_help();
		return 1;
	}

	if(argc==4)
		pass=argv[3];

	if(ifstr(argv[1],':')==1){
		if(formatpara(argv[1])){
			fprintf(stderr,"Parameter format error.\n");
			return 1;
		}
		recv_send=PULL;
		strpara(argv[1],pa,all_para);
		if(ifstr(argv[1],'@')){
			user=pa[0];
			host=pa[1];
			remote_path=pa[2];
		}else{
			host=pa[0];
			remote_path=pa[1];
		}
		local_path=argv[2];
	}else if(ifstr(argv[2],':')==1){
		if(formatpara(argv[2])){
			fprintf(stderr,"Parameter format error.\n");
			return 1;
		}
		recv_send=PUSH;
		strpara(argv[2],pa,all_para);
		if(ifstr(argv[2],'@')){
			user=pa[0];
			host=pa[1];
			remote_path=pa[2];
		}else{
			host=pa[0];
			remote_path=pa[1];
		}
		local_path=argv[1];
	}else{
		fprintf(stderr,"Parameter format error.\n");
		return 1;
	}

	printf("%s\t%s\t%s\t%s\n",user,pass,local_path,remote_path);
	return 0;

	//Host information analysis
	if(!ifip(host)){
		rc=sqlite3_checkinfo("ip",host);
		if(rc==0){
			ip=host;
		}else if(rc==2){
			fprintf(stderr,"Error:unknown ip address %s.",host);
			return 1;
		}
	}else if(!strcmp(host,"all"))
		all=host;
	else{
		rc=sqlite3_checkinfo("hostname",host);
		if(rc==2){
			rc=sqlite3_checkinfo("role",host);
			if(rc==2){
				fprintf(stderr,"Error:unknown host or role %s.",host);
				return 1;
			}else if(rc==0){
				role=host;
			}
		}else if(rc==0){
			hostname=host;
		}
	}
	if(rc==1){
		fprintf(stderr,"Error:database query failed.\n");
		return 1;
	}

	printf("%s\t%s\t%s\t%s\n",user,pass,local_path,remote_path);
	return 0;

	//GET table hostinfo and run scp
	int sql_ret=0;
	char **sql_result=init_Res();
	int sql_count=0;

	if(ip!=NULL){
		if(access(localfile))

	}else if(all!=NULL){
		//sqlite3_alltable(NULL,NULL,sql_result,&sql_count);
		fprintf(stderr,"Function temporarily unable to use\n");
	}else if(role!=NULL){
		//sqlite3_alltable("role",role,sql_result,&sql_count);
		fprintf(stderr,"Function temporarily unable to use\n");
	}else if(hostname!=NULL){
		sqlite3_alltable("hostname",hostname,sql_result,&sql_count);
	}


	return 0;
}

static int ifstr(char *ps,char pt){
	while(*ps){
		if(*ps == pt)
			return 1;
		ps++;
	}
	return 0;
}

static int formatpara(char *para){
	int ai=0;
	int aj=0;
	int mi=0;
	int mj=0;
	int k=0;
	while(*para){
		if(*para!='@')
			if(ai==k)
				ai++;
		if(*para=='@')
			aj++;
		if(*para!=':')
			if(mi==k)
				mi++;
		if(*para==':')
			mj++;
		k++;
		*para++;
	}
	if((ai!=0)&&(mi!=0)&&((ai==k)||((mi-ai)>1))&&((aj==1)||(aj==0))&&(mj==1))
		return 0;
	else
		return 1;
}

static int strpara(char *para,char *pd[],char pp[]){
        char *cp=pp;
        int i=1,j=0;
        pd[0]=&pp[0];

        while(*para){
                if((*para == '@') || (*para == ':')){
                        pp[j]='\0';
                        cp++;
                        j++;
                        pd[i]=&pp[j];
                        para++;
                        i++;
                        continue;
                }   
                *cp++=*para++;
                j++;
        }   
        pp[j]='\0';
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

static int ifdir(const char *path){
	struct stat st;
	stat(path,&st);
	if(S_ISDIR(st.st_mode))
		return 1;
	else if(S_ISREG(st.st_mode))
		return 0;
	else
		return -1;
}

static int ptype(char *path,int flag){
	char lp[1024];

	if(ifdir(path)==1){
		DIR *dir=NULL;
		struct dirent *entry;

		if((dir=opendir(path))==NULL)
			return 1;
		while(entry=readdir(dir))
			if(strcmp(entry->d_name,".")&&strcmp(entry->d_name,"..")){
				strcpy(lp,path);
				if(lp[strlen(lp)-1]=='/')
					lp[strlen(lp)-1]='\0';
				strcat(lp,"/");
				strcat(lp,etry->d_name);
				ptype(lp);
			}
	}else if(ifdir(path)==0){
		if(flag==PUSH)
			ssh_push(ip,user,pass,key,path,remotepath);
		else if(flag==PULL)
			ssh_pull(ip,user,pass,key,path,remotepath);
	}

	return 0;
}

static int _help(void){
	printf("Usage:\n\t");
	printf("Push file to remote host: \n\t\tvvscp localfile user@[host|ip|role|all]:filepath [password]\n\t");
	printf("Pull file from remote host: \n\t\tvvscp user@[host|ip|role|all]:filepath localfile [password]\n");
	printf("Options:\n\t");
	printf("host\t\t\tTarget host name\n\t");
	printf("ip\t\t\tTarget ipadder\n\t");
	printf("role\t\t\tTarget host role.All will be from the same role as host push or pull files\n\t");
	printf("all\t\t\tAll target host.All will be from the host push or pull files\n\t");
	printf("user\t\t\tTarget host user.role|all use the same user.user can set null\n\t");
	printf("password\t\tTarget host password.role|all use the same password\n\t");
	printf("role|all pull file localfile must directory\n");
	printf("Use option -H printf help info.\n");
	printf("For more see source code.\n");
	return 0;
}


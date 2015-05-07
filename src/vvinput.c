#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include "../lib/vvsqlite.h"

int main(int argc,char **argv){
	char *filename=NULL;
	if(argc == 2)
		filename=argv[1];
	else{
		fprintf(stderr,"Usage:%s filename\n",argv[0]);
		return 1;
	}

	

	FILE *fp=NULL;
	char *example="hostname|ip|user|password|private key|role\n\tor use /** Notes\nEx:\n\t"
		"localhost|127.0.0.1|root|123456|null|null\n\t"
		"localhost|127.0.0.1|root|null|/root/.ssh/id_rsa|null";
	if((fp=fopen(filename,"r"))==NULL){
		fprintf(stderr,"Cannot open file:%s\n",filename);
		return 1;
	}

	char *content=(char*)malloc(sizeof(char)*1024);
	char *hostinfo[10];
	int i,len;
	for(i=0;i<6;i++)
		hostinfo[i]=(char*)malloc(sizeof(char)*100);

	while((fgets(content,1024,fp))!=NULL){
		len=strlen(content);
		content[len-1]='\0';
		if(strstr(content,"/**") != NULL)
			continue;
		char *substr=strtok(content,"|");
		if(substr==NULL){
			fprintf(stderr,"file content format error:\n%s\n",example);
			continue;
		}	
		i=0;
		while(substr){
			strcpy(hostinfo[i++],substr);
			substr=strtok(NULL,"|");
		}
		if((i!=6) || (!strcmp(hostinfo[0],"null")) || (!strcmp(hostinfo[1],"null"))){
			fprintf(stderr,"file content format error:\n%s\n",example);
			return 0;
		}
		if(!strcmp(hostinfo[2],"null")){
			struct passwd *pwd;
			pwd=getpwuid(getuid());
			strcpy(hostinfo[2],pwd->pw_name);
		}
		if(!strcmp(hostinfo[3],"null")){
			strcpy(hostinfo[3],"null");
		}
		if(!strcmp(hostinfo[4],"null")){
			if(!strcmp(hostinfo[2],"root"))
				strcpy(hostinfo[4],"/root/.ssh/id_rsa");
			else{
				strcpy(hostinfo[4],"/home/");
				strcat(hostinfo[4],hostinfo[2]);
				strcat(hostinfo[4],"/.ssh/id_rsa");
			}
		}
		if(!strcmp(hostinfo[5],"null")){
			strcpy(hostinfo[5],"defalut");
		}
		
		if(sqlite3_insert(hostinfo[0],hostinfo[1],hostinfo[2],hostinfo[3],hostinfo[4],
					hostinfo[5]))
			fprintf(stderr,"Error: insert data failed\n");
		else
			printf("%s\t%s\t%s\t%s\t%s\t%s\tinsert successery!!!\n",hostinfo[0],hostinfo[1]
				,hostinfo[2],hostinfo[3],hostinfo[4],hostinfo[5]);
	}

	for(i=0;i<6;i++)
		free(hostinfo[i]);
	free(content);
	fclose(fp);
	return 0;
}

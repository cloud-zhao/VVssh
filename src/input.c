#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "vvsqlite.h"

int main(int argc,char **argv){
	char *filename=NULL;
	if(argc == 2)
		filename=argv[1];
	else{
		fprintf(stderr,"Usage:%s filename\n",argv[0]);
		return 1;
	}

	FILE *pfile=NULL;
	char *content=(char*)malloc(sizeof(char)*1024);
	char *example="hostname|ip|user|password|role\n\tor use /** Notes";
	if((fp=fopen(filename,"r"))==NULL){
		fprintf(stderr,"Cannot open file:%s\n",filename);
		return 1;
	}

	while(!feof(fp)){
		fgets(content,1024,fp);
		if(strstr(content,"/**") != NULL)
			continue;
		char *substr=strtok(content,"|");
		if(substr==NULL){
			fprintf(stderr,"file content format error:\n\t%s\n",example);
			continue;
		}	
		char *hostinfo[10];
		int i;
		for(i=0;i<5;i++)
			hostinfo[i]=(char*)malloc(sizeof(char));
		i=0;
		while(substr){
			strcpy(hostinfo[i++],substr);
			substr=strtok(NULL,"|");
		}
		if(i<3){
			fprintf(stderr,"file content format error:\n\t%s\n",example);
			continue;
		}
		if(i==3)
			strcpy(hostinfo[4],"default");
		i=sqlite3_insert(hostinfo[0],hostinfo[1],hostinfo[2],hostinfo[3],hostinfo[4]);
	}
}
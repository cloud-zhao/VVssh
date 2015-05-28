#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "vvsqlite.h"

int main(void){
	char datadir[1024];
	char datafile[1024];
	struct passwd *pwd;
	if(absolute_path(datafile))
		return 1;

	strcpy(datadir,datafile);
	datadir[strlen(datadir)-1]='\0';
	pwd=getpwuid(getuid());
	if(pwd==NULL){
		fprintf(stderr,"Get user error.\n");
		return 1;
	}else{
		strcat(datafile,pwd->pw_name);
		strcat(datafile,"_data.db");
	}

	if(!access(datafile,0)){
		printf("Data file already exists.\n");
		return 1;
	}
					
	if(access(datadir,0)){
		if(mkdir(datadir,S_IRWXU|S_IRWXG)){
			fprintf(stderr,"mkdir datadir failed.\n");
			return 1;
		}else
			printf("mkdir datadir successful.\n");
	}
	if(sqlite3_create())
		fprintf(stderr,"Database install failed.\n");
	else
		printf("Database install successful.\n");

	if(!access(datafile,0))
		chmod(datafile,S_IRUSR|S_IWUSR);
	if(!strcmp(pwd->pw_name,"root"))
		chmod(datadir,S_IRWXU|S_IRWXG|S_IRWXO);

	return 0;
}

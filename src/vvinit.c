#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../lib/vvsqlite.h"

int main(void){
	char *datadir="../data";
	char *datafile="../data/mydata.db";

	if(!access(datafile,0)){
		printf("Data file already exists.\n");
		return 0;
	}
	
	if(access(datadir,0)){
		if(mkdir(datadir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
			fprintf(stderr,"mkdir datadir failed.\n");
		else
			printf("mkdir datadir successful.\n");
	}
	if(sqlite3_create())
		fprintf(stderr,"Database install failed.\n");
	else
		printf("Database install successful.\n");
	return 0;
}

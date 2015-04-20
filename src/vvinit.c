#include <stdio.h>
#include "../lib/vvsqlite.h"

int main(void){
	if(sqlite3_create())
		fprintf(stderr,"Database install failed.\n");
	else
		printf("Database install successful.\n");
	return 0;
}

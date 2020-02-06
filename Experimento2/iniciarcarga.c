#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NO_OF_CHILDREN 5


int main()
{

	int rtn;
	int count;

	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
			rtn = fork();
		} else {
			break;
		}
	}


	if(rtn == 0)
	{
		char * argv_list[] = {NULL};	
		execv("./carga",argv_list); 
	}
	else
	{
		for(count = 0;count<NO_OF_CHILDREN;count++)
		{
			wait(NULL);
		}
	} 
	exit(0);
	

}

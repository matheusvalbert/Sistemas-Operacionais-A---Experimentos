/*
Alexandre Gil Lujan 			17148826
Gabriel Albertin dos Santos 	17214172
Gabriela Nelsina Vicente 		17757089
Matheus Valbert 				17055641
Tomas Martinolli Verwiebe 		17108978
*/

#include <stdlib.h>
#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday() and fork() */
#include <stdio.h>		/* for printf() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <string.h>
  
int main(int argc, char *argv[ ]) 
{ 

	struct timeval start_time;
	struct timeval stop_time;
	float drift;
	int count;
  	int i; 
	char numero_child[10], numero_interacao[10], numero_sleep[10],numero_micro[10];
	strcpy(numero_child,argv[0]);
	strcpy(numero_interacao,argv[1]);
	strcpy(numero_sleep,argv[2]);
	strcpy(numero_micro,argv[3]);
	int no_child, no_int, no_sleep, no_micro;
	no_child = atoi(numero_child);
	no_int = atoi(numero_interacao);
	no_sleep = atoi(numero_sleep);
	no_micro = atoi(numero_micro);
	
	
	gettimeofday( &start_time, NULL );

		
	for( count = 0; count < no_int; count++ ) {
		usleep(no_sleep);
	}

	gettimeofday( &stop_time, NULL );


	drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
	drift += (stop_time.tv_usec - start_time.tv_usec)/(float)no_micro;


	printf("Filho #%d -- desvio total: %.4f -- desvio medio: %.4f\n",no_child+1, drift - no_int*no_sleep/no_micro,(drift - no_int*no_sleep/no_micro)/no_int);
		
      
    return 0; 
} 

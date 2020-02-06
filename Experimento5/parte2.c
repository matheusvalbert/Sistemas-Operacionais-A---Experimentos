#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <string.h>
#include <time.h>


#define NUM_OF_CHAIRS 7
#define NUM_OF_CUSTOMERS 27
#define NUM_OF_BARBERS 3
#define SEM_KEY		0x1291
#define SEM_KEY2	0x1292
#define MICRO_PER_SECOND	1000000

pthread_t barbeiros[NUM_OF_BARBERS];
pthread_t clientes[NUM_OF_CUSTOMERS];
pthread_mutex_t exclusao;
pthread_mutex_t exclusao_atende;
pthread_mutex_t exclusao_resultado;
int waiting;
int g_sem_customers;
int g_sem_barbers;
char string[27][5000];
char resultado[27][5000];
int atendido[NUM_OF_CUSTOMERS];
int countBarbeiro = 0, countCliente = 0;
int atende;


struct sembuf	up[1];
struct sembuf	down[1];




void *func_barbeiro(void *threadid) {

	int id_barbeiro = *(int *) threadid;
	id_barbeiro += 1;
	
	int vetor[1024];
	char vetcopia[5000];
	char vetresult[5000];
	int i;
	int k=0;
	int j=0;
	int aux;
	char c;
	char number[10];
	int atendendo;
	int flag = 0;

	while(1 == 1)
	{
		flag = 0;
		atendendo=0;
		for(i=0;i<1024;i++)
			vetor[i]=0;
		j=0;
		vetcopia[0] = '\0';
		vetresult[0]='\0';
		if( semop( g_sem_customers, down, 1 ) == -1 ) {
			exit(1);
		}
		pthread_mutex_lock(&exclusao);
		waiting --;
		while(flag == 0)
		{
			pthread_mutex_lock(&exclusao_atende);
			if(atende !=-1)
			{
				atendendo = atende;
				atende = -1;
				flag = 1;
				strcpy(vetcopia,string[atendendo]);
				atendido[atendendo] = id_barbeiro;
			}
			pthread_mutex_unlock(&exclusao_atende);
		}
		pthread_mutex_unlock(&exclusao);
												//cut hair
		for(i=0;vetcopia[i]!='\0';i++)
		{	
			c = vetcopia[i];
			if(c==' ')
			{
				j++;
			}
			else
				vetor[j] = 10*vetor[j] + (c - 48);
		}
		for(i=0;i<j+1;i++)
		{
			for(k=0;k<j+1;k++)
			{
				if(vetor[i] < vetor[k])
				{
					aux = vetor[i];
					vetor[i] = vetor[k];
					vetor[k] = aux;
				}
			}
		}
		sprintf(number,"%d",vetor[0]);
		strcpy(vetresult,number);
		number[0] = '\0';
		for(i=1;i<j+1;i++)
		{
			sprintf(number," %d",vetor[i]);
			strcat(vetresult,number);
			number[0]='\0';
		}
		
		pthread_mutex_lock(&exclusao_resultado);
		strcpy(resultado[atendendo],vetresult);
		pthread_mutex_unlock(&exclusao_resultado);
		
		if( semop( g_sem_barbers, up, 1 ) == -1 ) {
			fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
			exit(1);	
		}
	}
	

}

void *func_cliente(void *threadid) {
	
	int id_cliente = *(int *) threadid;
	char stringcliente[1023*5+1];
	int num;
	int numloop;
	int i;
	int flag = 0,flag2 = 0,flag3 = 0;
	char numero[10];
	struct timeval tv1, tv2;
	float number;
	while(flag == 0)
	{
		pthread_mutex_lock(&exclusao);
		if(waiting < NUM_OF_CHAIRS)
		{
			if( gettimeofday( &tv1, NULL ) == -1 ) {
				fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
				exit(1);
			}
			waiting++;
			if( semop( g_sem_customers, up, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
				exit(1);
			}
			pthread_mutex_unlock(&exclusao);
			num = rand() % 1000;
			sprintf(numero,"%d",num);
			strcpy(stringcliente,numero);
			numloop = rand()%1021 + 1;
			numero[0] = '\0';
			for(i=0;i<numloop;i++)
			{
				num = rand() % 1000;
				sprintf(numero," %d",num);
				strcat(stringcliente,numero);
				numero[0] = '\0';
			}
			while(flag2 == 0)
			{
				pthread_mutex_lock(&exclusao_atende);
				if(atende == -1)
				{
					atende = id_cliente;
					flag2 = 1;
					strcpy(string[id_cliente],stringcliente);
				}
				pthread_mutex_unlock(&exclusao_atende);
			}
			
			if( semop( g_sem_barbers, down, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
				exit(1);
			}

			while(flag3==0)
			{
				pthread_mutex_lock(&exclusao_resultado);
				if(strlen(resultado[id_cliente]) != 0)
				{														//appreciate hair
					if( gettimeofday( &tv2, NULL ) == -1 ) {
						fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
						exit(1);
					}
					number = (tv2.tv_usec - tv1.tv_usec)/(float)MICRO_PER_SECOND;
					printf("Atendimento:\n cliente: %i - barbeiro: %i - tempo de atendimento: %f\n vetor desorganizado: %s\n vetor organizado   : %s\n",id_cliente, atendido[id_cliente],number,stringcliente,resultado[id_cliente]);
					flag3 = 1;
				}
				pthread_mutex_unlock(&exclusao_resultado);
			}
			flag = 1;
		}
		else
		{
			pthread_mutex_unlock(&exclusao);
			num = rand() % 10000;
			usleep(num);
		}
	}
}

int main () {
	
	int i, tc;
	void *ret;
	waiting = 0;
	time_t t;
	atende = -1;
	srand((unsigned) time(&t));

	down[0].sem_num   =  0;
	down[0].sem_op    = -1;
	down[0].sem_flg   =  0;

	up[0].sem_num =  0;
	up[0].sem_op  =  1;
	up[0].sem_flg =  0;

	if( ( g_sem_customers = semget( SEM_KEY, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}


	if( ( g_sem_barbers = semget( SEM_KEY2, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}



	if (pthread_mutex_init(&exclusao, NULL) != 0) 
	{ 
	    printf("falha iniciacao semaforo\n");
	    return 1; 
	}
	if (pthread_mutex_init(&exclusao_atende, NULL) != 0) 
	{ 
	    printf("falha iniciacao semaforo\n");
	    return 1; 
	}
	if (pthread_mutex_init(&exclusao_resultado, NULL) != 0) 
	{ 
	    printf("falha iniciacao semaforo\n");
	    return 1; 
	}
	
	
	for (i=0;i<NUM_OF_BARBERS;i++) {
    
    	tc = pthread_create(&barbeiros[i], NULL, func_barbeiro, &i);

	    if (tc) {
	      printf("ERRO: impossivel criar um thread consumidor\n");
	      exit(-1);
	    }
	    
	    usleep(1000);
	}

	for (i=0;i<NUM_OF_CUSTOMERS;i++) {
    
    	tc = pthread_create(&clientes[i], NULL, func_cliente, &i);

	    if (tc) {
	      printf("ERRO: impossivel criar um thread consumidor\n");
	      exit(-1);
	    }

	    usleep(1000);
	}
	
	
// TÉRMINOS


	for(i=0;i < NUM_OF_CUSTOMERS;i++) {

		pthread_join(clientes[i], &ret);
	}
		

	 pthread_mutex_destroy(&exclusao);
	 pthread_mutex_destroy(&exclusao_atende);
	 pthread_mutex_destroy(&exclusao_resultado);
	 
	 if( semctl( g_sem_customers, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

	 if( semctl( g_sem_barbers, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

	exit(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <wait.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#define NO_OF_CHILDREN 	22
#define MESSAGE_QUEUE_ID 3547
#define MESSAGE_QUEUE_ID_2 3857
#define MICRO_PER_SECOND 1000000
#define MESSAGE_MTYPE	1
#define SEM_KEY		0x1276
#define NUM_OF_CHAIRS   7
#define SHM_KEY		0x1442


int g_sem_exclusao;
int	g_shm_id;
int	*g_shm_addr;


struct sembuf	up[1];
struct sembuf	down[1];

typedef struct {

	long mtype;
	char message[5000];
} bufmsg;

typedef struct {

	long mtype;
	char string[5000];
	struct timeval tv2;
	int numero_barbeiro;
} bufmsg_devolve;

void barbeiro(int count, int queue_id, int queue_id_2) {

	int vetor[1023];
	int i;
	int k=0;
	int j=0;
	int aux;
	char c;
	char resultado[5000];
	char numero[10];
	int waiting;
	
	bufmsg message;
	bufmsg_devolve message_dev;
	
	while(1==1)
	{

		for(i=0;i<1023;i++)
			vetor[i] = 0;
		resultado[0]='\0';
		j = 0;
		if( msgrcv(queue_id,(struct msgbuf *)&message,sizeof(bufmsg), MESSAGE_MTYPE,0) == -1 ) {
			exit(1);
		}
			
		if( gettimeofday( &message_dev.tv2, NULL ) == -1 ) {
				fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
				exit(1);
		}
	
		if( semop( g_sem_exclusao, down, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
		}
		waiting = *g_shm_addr;
		waiting--;
		*(g_shm_addr) = waiting;
		

		if( semop( g_sem_exclusao, up, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
		}

		for(i=0;i<10;i++)
			vetor[i]=0;
		for(i=0;message.message[i]!='\0';i++)
		{	
			c = message.message[i];
			if(c==' ')
				j++;
			else
				vetor[j] = 10*vetor[j] + (c - 48);
		}
		for(i=0;i<j+1;i++)
			for(k=0;k<j+1;k++)
			{
				if(vetor[i] > vetor[k])
				{
					aux = vetor[i];
					vetor[i] = vetor[k];
					vetor[k] = aux;
				}
			}

		for(i=0;i<j+1;i++) {
		
			sprintf(numero," %d",vetor[i]);
			strcat(resultado,numero);
		}


		message_dev.mtype = MESSAGE_MTYPE;

		strcpy(message_dev.string, resultado);

		message_dev.numero_barbeiro = count;

		if(msgsnd(queue_id_2,(struct msgbuf*)&message_dev,sizeof(bufmsg_devolve),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
	}

	return;

}

void cliente(int count, int queue_id, int queue_id_2) {

	int waiting;
	time_t  t;
	char numero[10];
	char stringcliente[5000];
	int numloop, num;
	int i;
	float number;
	struct timeval tv1;

	bufmsg message;
	bufmsg_devolve message_dev;
	if( semop( g_sem_exclusao, down, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
	}
	waiting = *g_shm_addr;
	if(waiting < NUM_OF_CHAIRS)
	{
		if( gettimeofday( &tv1, NULL ) == -1 ) {
				fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
				exit(1);
		}	
		waiting++;
		*(g_shm_addr) = waiting;
		
		if( semop( g_sem_exclusao, up, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
		}
		
		message.mtype = MESSAGE_MTYPE;

		srand(time(NULL) ^ (getpid()<<16));
		numloop = 2 + rand() % 1021;

		for(i=0;i<numloop;i++)
			{
				num = rand() % 1000;
				sprintf(numero," %d",num);
				strcat(message.message,numero);
			}	


		if(msgsnd(queue_id,(struct msgbuf*)&message,sizeof(bufmsg),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}

		if( msgrcv(queue_id_2,(struct msgbuf *)&message_dev,sizeof(bufmsg_devolve), MESSAGE_MTYPE,0) == -1 ) {
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

		number = (message_dev.tv2.tv_usec - tv1.tv_usec)/(float)MICRO_PER_SECOND;

		printf("\n\nNumero Cliente: %i - Numero Barbeiro: %i Tempo: %f\nString: %s\n", count + 1, message_dev.numero_barbeiro, number, message_dev.string);
	}
	else
	{
		printf("Cliente %i nao foi atendido\n",count+1);
		if( semop( g_sem_exclusao, up, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
	}
	}
	return;
}

int main () {

	int count, rtn = 1;
	key_t key = MESSAGE_QUEUE_ID;
	key_t key2 = MESSAGE_QUEUE_ID_2;
	int queue_id, queue_id_2;

	down[0].sem_num   =  0;
	down[0].sem_op    = -1;
	down[0].sem_flg   =  0;

	up[0].sem_num =  0;
	up[0].sem_op  =  1;
	up[0].sem_flg =  0;

	if( ( g_sem_exclusao = semget( SEM_KEY, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if( semop( g_sem_exclusao, up, 1 ) == -1 ) {
				fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo! cliente 1");
				exit(1);
	}

	if( (g_shm_id = shmget( SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	*g_shm_addr = 0;
	
	/*
         * Cria a fila de mensagens
         */
        if( (queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	if( (queue_id_2 = msgget(key2, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	
    for( count = 0; count < NO_OF_CHILDREN; count++ ) {
        if( rtn != 0 ) {
        	rtn = fork();
        } 
        else {
            break;
        }
    }

    if(rtn == 0 && count < 3) {
	barbeiro(count, queue_id, queue_id_2);
    }

    else if (rtn == 0) {

	cliente((count - 3), queue_id, queue_id_2);
    }

    else {

    	for(count = 0; count < NO_OF_CHILDREN-2; count++) {

    		wait(NULL);
    	}

	if( msgctl(queue_id,IPC_RMID,NULL) == -1 ) {
		fprintf(stderr,"Impossivel remover a fila!\n");
		exit(1);
	}

	if( msgctl(queue_id_2,IPC_RMID,NULL) == -1 ) {
		fprintf(stderr,"Impossivel remover a fila!\n");
		exit(1);
	}

	 if( semctl( g_sem_exclusao, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
        }

	if( shmctl(g_shm_id,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

    }


	exit(0);
}

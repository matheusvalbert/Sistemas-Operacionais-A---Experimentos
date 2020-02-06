/***********************************************************************************
*
* Este programa não faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* embora tenha sido inspirado pelos demais experimentos lah existentes.
*
* Experimento # 4 na disciplina de Sistemas Operacionais da PUC-Campinas
* Originalmente programado por Florian Weizenegger
*							Data: 25/08/2003
* 
*       Proposito: O proposito deste programa e o de permitir ao aluno perceber
*       o que vem a ser um thread, de maneira tal que consiga distingui-lo de
*       um processo. Além disso, são usados os principais comandos para criação
*	e manipulação de threads.
*	O problema dos produtores e consumidores sobre um buffer circular é
*	usado como assunto, permitindo que o aluno experimente duas implementações
*	diferentes para sua solução. Desta maneira, além dos threads propriamente
*	ditos, tambem locks e semaforos sao usados para garantir sincronizacao
*	de threads.
*
*************************************************************************************/

/*
 * Includes Necessarios 
 */

#include <pthread.h>			/* para poder manipular threads */
#include <stdio.h>			/* para printf() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Constantes Necessarias 
 */
#define NUM_THREADS     5
#define NO_OF_ITERATIONS 365

/*
 * O tipo pthread_t permite a declaração de uma variável que recebe
 * um id quando o thread é criado. Posteriormente, esse id pode ser
 * usado em comandos de controle para threads.
 * Seguem dois vetores para ids, para um numero de threads igual a
 * constante NUM_THREADS
 */
pthread_t filosofos[NUM_THREADS];
pthread_mutex_t talheres[NUM_THREADS];
int vetor[5];


/*
 * Variaveis Necessarias 
 */

/*
 * A rotina produce e responsavel por chamar myadd para que seja 
 * colocado o valor 10 em uma posicao do buffer NO_OF_ITERATIONS vezes
 */
void *filosofo(void *threadid)
{

 int i = 0;
 int j =0;
 int id = *(int *) threadid;
 char mensg[100];
 char c[5];
 //printf("Inicio do filosofo - %i\n" , id);

while(i < NO_OF_ITERATIONS) {

  	pthread_mutex_lock(&talheres[id]);
    if(pthread_mutex_trylock(&talheres[(id+1)%NUM_THREADS]) == 0) {
	vetor[id] = 1;
    	usleep(25);
	sprintf(c, "%d ", id);
	strcpy(mensg,"O filosofo ");
	strcat(mensg,c);
	strcat(mensg," conseguiu pensar - ");
	strcat(mensg,"Filofos pensando :");
	for(j=0;j<NUM_THREADS;j++)
	{
		if(vetor[j] == 1)
		{
			sprintf(c, "%d ", j);
			strcat(mensg,c)	;
		}
			
	}
	strcat(mensg," - Filofos nao pensando :");
	for(j=0;j<NUM_THREADS;j++)
	{
		if(vetor[j] == 0)
		{
			sprintf(c, "%d ", j);
			strcat(mensg,c)	;
		}
			
	}
	strcat(mensg,"\n");
	printf("%s", mensg);
	vetor[id] = 0;

 
      i++;
      pthread_mutex_unlock(&talheres[(id+1)%NUM_THREADS]);
      pthread_mutex_unlock(&talheres[id]);
    }
    else {

      pthread_mutex_unlock(&talheres[id]);
	sprintf(c, "%d ", id);
	strcpy(mensg,"O filosofo ");
	strcat(mensg,c);
	strcat(mensg," tentou pensar - ");
	strcat(mensg,"Filosofos pensando :");
	for(j=0;j<NUM_THREADS;j++)
	{
		if(vetor[j] == 1)
		{
			sprintf(c, "%d ", j);
			strcat(mensg,c)	;
		}
			
	}
	strcat(mensg," - Filosofos nao pensando :");
	for(j=0;j<NUM_THREADS;j++)
	{
		if(vetor[j] == 0)
		{
			sprintf(c, "%d ", j);
			strcat(mensg,c)	;
		}
			
	}
	strcat(mensg,"\n");
	printf("%s", mensg);	
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  int tc;
  int i;

  void *ret;



 if (pthread_mutex_init(&talheres[0], NULL) != 0) 
    { 
        printf("falha iniciacao semaforo\n");
        return 1; 
    }

 if (pthread_mutex_init(&talheres[1], NULL) != 0) 
    { 
        printf("falha iniciacao semaforo\n");
        return 1; 
    }

 if (pthread_mutex_init(&talheres[2], NULL) != 0) 
    { 
        printf("falha iniciacao semaforo\n");
        return 1; 
    }
 if (pthread_mutex_init(&talheres[3], NULL) != 0) 
    { 
        printf("falha iniciacao semaforo\n");
        return 1; 
    }
 if (pthread_mutex_init(&talheres[4], NULL) != 0) 
    { 
        printf("falha iniciacao semaforo\n");
        return 1;
    }
  

	for(i=0;i<NUM_THREADS;i++)
		vetor[i] = 0;

  for (i=0;i<NUM_THREADS;i++) {

    // tenta criar um thread consumidor
    tc = pthread_create(&filosofos[i], NULL, filosofo, &i);

    if (tc) {
      printf("ERRO: impossivel criar um thread consumidor\n");
      exit(-1);
    }
    usleep(250);
  }
 for(i=0;i<NUM_THREADS;i++)
 {
    pthread_join(filosofos[i], &ret);
 }

 pthread_mutex_destroy(&talheres[0]);
 pthread_mutex_destroy(&talheres[1]);
 pthread_mutex_destroy(&talheres[2]);
 pthread_mutex_destroy(&talheres[3]);
 pthread_mutex_destroy(&talheres[4]);

  printf("Terminando a thread main()\n");
  pthread_exit(NULL);
}

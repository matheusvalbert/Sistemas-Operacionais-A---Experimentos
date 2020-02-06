/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #5: Semaphores
*
*    Programmer: Eric Sorton
*          Date: 3/17/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to demonstrate how semaphores
*		 can be used to protect a critical region.  Its sole purpose
*		 is to print a character string (namely the alphabet) to the
*		 screen.  Any number of processes can be used to cooperatively
*		 (or non-cooperatively) print the string to the screen.  An
*		 index is stored in shared memory, this index is the index into
*		 the array that identifies which character within the string
*		 should be printed next.  Without semaphores, all the processes
*		 access this index simultaneously and conflicts occur.  With
*		 semahpores, the character string is displayed neatly to the
*		 screen.
*
*		 The optional semaphore protection can be compiled into the
*		 program using the MACRO definition of PROTECT.  To compile
*		 the semaphore protection into the program, uncomment the
*		 #define below.
*
*
*       Proposito: O proposito deste programa e o de demonstrar como semaforos
*		podem ser usados para proteger uma regiao critica. O programa exibe
*		um string de caracteres (na realidade um alfabeto). Um número 
*		qualquer de processos pode ser usado para exibir o string, seja
*		de maneira cooperativa ou nao cooperativa. Um indice e armazenado
*		em memoria compartilhada, este indice e aquele usado para 
* 		identificar qual caractere deve ser exibido em seguida. Sem 
*		semaforos, todos os processos acessam esse indice concorrentemente 
*		causando conflitos. Com semaforos, o string de caracteres e exibido
*		de maneira correta (caracteres do alfabeto na ordem correta e apenas
*		um de cada caractere).
*
*		A protecao opcional com semaforo pode ser compilada no programa
*		usando a definicao de MACRO denominada PROTECT. Para compilar a
*		protecao com semaforo, retire o comentario do #define que segue.
*
*
*******************************************************************************/



#define PROTECT



/*
 * Includes Necessarios 
 */
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <stdio.h>              /* for printf() */
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <stdlib.h>
#include <string.h>


/*
 * Constantes Necessarias 
 */
#define SEM_KEY		0x1243
#define SEM_KEY2	0x1248
#define SEM_KEY3	0x1260
#define SHM_KEY		0x1432
#define SHM_KEY2	0x1333
#define SHM_KEY3	0x1599
#define SHM_KEY4	0x1230
#define NO_OF_CHILDREN	8

#define KEY1	0x0436
#define KEY2	0x0537
#define KEY3	0x0638
#define KEY4	0x0739


/*
 * As seguintes variaveis globais contem informacao importante. A variavel
 * g_sem_id e g_shm_id contem as identificacoes IPC para o semaforo e para
 * o segmento de memoria compartilhada que sao usados pelo programa. A variavel
 * g_shm_addr e um ponteiro inteiro que aponta para o segmento de memoria
 * compartilhada que contera o indice inteiro da matriz de caracteres que contem
 * o alfabeto que sera exibido.
*/
int	g_sem_id_prod;
int 	g_sem_id_cons;
int	g_sem_id_print;
int	g_shm_id_1;
int 	g_shm_id_2;
int 	g_shm_string;
char 	*strPont;
int	*g_shm_addr_prod;
int 	*g_shm_addr_cons;
int 	g_shm_id_count;
int	*g_shm_addr_count;
int	mem1, mem2, mem3, mem4;
char 	*pont1, *pont2, *pont3, *pont4;
/*
 * As seguintes duas estruturas contem a informacao necessaria para controlar
 * semaforos em relacao a "fecharem", se nao permitem acesso, ou 
 * "abrirem", se permitirem acesso. As estruturas sao incializadas ao inicio
 * do programa principal e usadas na rotina PrintAlphabet(). Como elas sao
 * inicializadas no programa principal, antes da criacao dos processos filhos,
 * elas podem ser usadas nesses processos sem a necessidade de nova associacao
 * ou mudancas.
*/
struct sembuf	g_sem_op1[1];
struct sembuf	g_sem_op2[1];

/*
 * O seguinte vetor de caracteres contem o alfabeto que constituira o string
 * que sera exibido.
*/
char g_letters_and_numbers[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";

/*
 * Funcoes
 */
void Produtor( int num );
void Consumidor( void );

/*
 * Programa Principal
 */
int main( int argc, char *argv[] )
{
      /*
       * Variaveis necessarias
       */
      int rtn;
      int count;
      int numero = 0;
        
	int i,j;

      /*
       * Para armazenar os ids dos processos filhos, permitindo o posterior
       * uso do comando kill
       */
      int pid[NO_OF_CHILDREN];

	/*
	 * Construindo a estrutura de controle do semaforo
	 */
	g_sem_op1[0].sem_num   =  0;
	g_sem_op1[0].sem_op    = -1;
	g_sem_op1[0].sem_flg   =  0;

	/* 
	 * Pergunta 1: Se usada a estrutura g_sem_op1 terá qual efeito em um conjunto de semáforos?
	 */

	g_sem_op2[0].sem_num =  0;
	g_sem_op2[0].sem_op  =  1;
	g_sem_op2[0].sem_flg =  0;

	/*
	 * Criando o semaforo
	 */	
	if( ( g_sem_id_prod = semget( SEM_KEY, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	
	if( semop( g_sem_id_prod, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	if( ( g_sem_id_cons = semget( SEM_KEY2, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	
	if( semop( g_sem_id_cons, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	if( ( g_sem_id_print = semget( SEM_KEY3, 1, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	
	if( semop( g_sem_id_print, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	/* 
	 * Pergunta 2: Para que serve esta operacao semop(), se não está na saída de uma região crítica?
	 */

	/*
	 * Criando o segmento de memoria compartilhada
	 */
	if( (g_shm_id_1 = shmget( SHM_KEY, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_id_2 = shmget( SHM_KEY4, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_string = shmget( SHM_KEY2, 66*sizeof(char), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (strPont = (char *)shmat(g_shm_string, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr_prod = (int *)shmat(g_shm_id_1, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	*g_shm_addr_prod = 0;

	if( (g_shm_addr_cons = (int *)shmat(g_shm_id_2, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	*g_shm_addr_cons = 0;

	if( (g_shm_id_count = shmget( SHM_KEY3, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr_count = (int *)shmat(g_shm_id_count, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	*g_shm_addr_count = 0;

	/////////////////////////////////////////

	if( (mem1 = shmget( KEY1, 100*sizeof(char), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (pont1 = (char *)shmat(mem1, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	if( (mem2 = shmget( KEY2, 100*sizeof(char), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (pont2 = (char *)shmat(mem2, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	if( (mem3 = shmget( KEY3, 100*sizeof(char), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	if( (pont3 = (char *)shmat(mem3, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	
	if( (mem4 = shmget( KEY4, 100*sizeof(char), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (pont4 = (char *)shmat(mem4, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	/*
	 * Pergunta 3: Para que serve essa inicialização da memória compartilhada com zero?
	 */

       /*
        * Criando os filhos
        */
       rtn = 1;
       for( count = 0; count < NO_OF_CHILDREN; count++ ) {
               if( rtn != 0 ) {
                       pid[count] = rtn = fork();
               } else {
                       break;
               }
       }
	if(count %2 == 1);	       
		    numero = count/2;

       /*
        * Verificando o valor retornado para determinar se o processo e 
        * pai ou filho 
        */
       if( rtn == 0 && count %2 ==1 ) {
                /*
                 * Eu sou um filho
                 */
                printf("Filho %i comecou - Produtor ...\n", count);		

		Produtor(numero);

        } else if(rtn == 0 ) {
	
		//usleep(1);
		printf("Filho %i comecou - Consumidor ...\n", count);
		
		Consumidor();

	
	}else{

                usleep(30000);

                /*
                 * Matando os filhos 
                 */
                kill(pid[0], SIGKILL);
                kill(pid[1], SIGKILL);
                kill(pid[2], SIGKILL);
		kill(pid[3], SIGKILL);
                kill(pid[4], SIGKILL);
                kill(pid[5], SIGKILL);
		kill(pid[6], SIGKILL);
                kill(pid[7], SIGKILL);

		
		printf("\nProdutor 1 - %s\n", pont1);
		printf("Produtor 2 - %s\n", pont2);
		printf("Produtor 3 - %s\n", pont3);
		printf("Produtor 4 - %s\n", pont4);

                /*
                 * Removendo a memoria compartilhada
                 */
                if( shmctl(g_shm_id_1,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(g_shm_id_2,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

                if( shmctl(g_shm_string,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(g_shm_id_count,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(mem1,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(mem2,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(mem3,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(mem4,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

                /*
                 * Removendo o semaforo
                 */
                if( semctl( g_sem_id_prod, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

                 if( semctl( g_sem_id_cons, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

		if( semctl( g_sem_id_print, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

                shmdt(strPont);

                exit(0);
        }
}

	/*
	* Pergunta 4: se os filhos ainda não terminaram, semctl e shmctl, com o parametro IPC-RMID, nao
	* permitem mais o acesso ao semáforo / memória compartilhada?
	*/

/*
 * Esta rotina realiza a exibicao de caracteres. Nela e calculado um numero
 * pseudo-randomico entre 1 e 3 para determinar o numero de caracteres a exibir.
 * Se a protecao esta estabelecida, a rotina entao consegue o recurso. Em
 * seguida, PrintChars() acessa o indice com seu valor corrente a partir da
 * memoria compartilhada. A rotina entra em loop, exibindo o numero aleatorio de
 * caracteres. Finalmente, a rotina incrementa o indice, conforme o necessario,
 * e libera o recurso, se for o caso.
*/
void Produtor(int num )
{
	struct timeval tv;
	int number;
	int tmp_index;
	int tmp_count;
	int i;
	int count = 0;

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(800);

	/*
	 * Entrando no loop principal
	 */
	while(1) {

		/*
                 * Conseguindo o tempo corrente, os microsegundos desse tempo
             	 * sao usados como um numero pseudo-randomico. Em seguida,
            	 * calcula o numero randomico atraves de um algoritmo simples
		 */

#ifdef PROTECT
		if( semop( g_sem_id_prod, g_sem_op1, 1 ) == -1 ) {
                       	fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso!");
                       	exit(1);
               	 }
#endif

		/*
		 * Pergunta 5: quais os valores possíveis de serem atribuidos 
		 * a number?
		 */
#ifdef PROTECT
		if( semop( g_sem_id_print, g_sem_op1, 1 ) == -1 ) {
			fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
			exit(1);
		}
#endif

		/*
		 * O #ifdef PROTECT inclui este pedaco de codigo se a macro
            * PROTECT estiver definida. Para sua definicao, retire o comentario
            * que a acompanha. semop() e chamada para fechar o semaforo.
            */


		/*
		 * Lendo o indice do segmento de memoria compartilhada
		 */

		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}


		number = ((tv.tv_usec / 47) % 5) + 1;
		tmp_index = *g_shm_addr_prod;
		tmp_count = *g_shm_addr_count;


		if((tmp_index + number)>65)
			number = 65 - tmp_index;

		/*
            * Repita o numero especificado de vezes, esteja certo de nao
            * ultrapassar os limites do vetor, o comando if garante isso
		 */
		for( i = 0; i < number; i++ ) {
			if( ! (tmp_index + i > 	sizeof(g_letters_and_numbers))) {
				strPont[tmp_index + i] = g_letters_and_numbers[tmp_index + i];
				//fprintf(stderr,"%c", strPont[tmp_index + i]);
				usleep(1);
				if(num == 0) {
					pont1[count]=strPont[tmp_index + i];
				}
				else if (num == 1) {
					pont2[count]=strPont[tmp_index + i];
				}
				else if (num == 2) {
					pont3[count]=strPont[tmp_index + i];
				}
				else {
					pont4[count]=strPont[tmp_index + i];
				}
				count++;
			}
		}

		if(num == 0) {
			pont1[count] = '\0';
		}
		else if (num == 1) {
			pont2[count] = '\0';
		}
		else if (num == 2) {
			pont3[count] = '\0';
		}
		else {
			pont4[count] = '\0';
		}


		usleep(number);
		/*
		 * Atualizando o indice na memoria compartilhada
		 */

		*g_shm_addr_prod = tmp_index + i;

		*g_shm_addr_count = tmp_count + number;

		/*
         	 * Se o indice e maior que o tamanho do alfabeto, exibe um
         	 * caractere return para iniciar a linha seguinte e coloca
         	 * zero no indice
		 */
		if( tmp_index + i >= 65 ) {
			printf("Produtor   -%s\n", strPont);
			*g_shm_addr_prod = 0;
		}
		//printf("Produtor   %2i-%s\n",*g_shm_addr_count, strPont);

		/*
		 * Liberando o recurso se a macro PROTECT estiver definida
		 */

#ifdef PROTECT
		if( semop( g_sem_id_print, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif


#ifdef PROTECT
		if( semop( g_sem_id_prod, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif

	}
}






void Consumidor( void )
{
	struct timeval tv;
	int number;

	int tmp_index;
	int tmp_count;
	int i;

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(800);

	/*
	 * Entrando no loop principal
	 */
	while(1) {

		/*
                 * Conseguindo o tempo corrente, os microsegundos desse tempo
             	 * sao usados como um numero pseudo-randomico. Em seguida,
            	 * calcula o numero randomico atraves de um algoritmo simples
		 */
		/*
		 * Pergunta 5: quais os valores possíveis de serem atribuidos 
		 * a number?
		 */

		/*
		 * O #ifdef PROTECT inclui este pedaco de codigo se a macro
            * PROTECT estiver definida. Para sua definicao, retire o comentario
            * que a acompanha. semop() e chamada para fechar o semaforo.
            */

#ifdef PROTECT
		if( semop( g_sem_id_cons, g_sem_op1, 1 ) == -1 ) {
                       	fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso!");
                       	exit(1);
               	 }
#endif



#ifdef PROTECT
		if( semop( g_sem_id_print, g_sem_op1, 1 ) == -1 ) {
			fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
			exit(1);
		}
#endif

		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}
		number = ((tv.tv_usec / 47) % 5) + 1;

		tmp_count = *g_shm_addr_count;
		if(tmp_count != 0)
		{
			if(tmp_count < number)
				number = tmp_count;

			/*
			 * Lendo o indice do segmento de memoria compartilhada
			 */
			tmp_index = *g_shm_addr_cons;
	
			if((tmp_index + number)>65)
			{
				number = 65 - tmp_index;
			}


			/*
		    * Repita o numero especificado de vezes, esteja certo de nao
		    * ultrapassar os limites do vetor, o comando if garante isso
			 */
		
			for( i = 0; i < number; i++ ) {
				if( ! (tmp_index + i > 	sizeof(g_letters_and_numbers)) ) {
					strPont[tmp_index + i] = '#';
					usleep(1);
				}
			}
			usleep(number);
			/*
			 * Atualizando o indice na memoria compartilhada
			 */

			*g_shm_addr_cons = tmp_index + i;
			*g_shm_addr_count = tmp_count - number;

			/*
		 	 * Se o indice e maior que o tamanho do alfabeto, exibe um
		 	 * caractere return para iniciar a linha seguinte e coloca
		 	 * zero no indice
			 */
			if( tmp_index + i >= 65 ) {
				printf("Consumidor -%s\n", strPont);
				*g_shm_addr_cons = 0;
			}
			//printf("Consumidor %2i-%s\n",*g_shm_addr_count, strPont);
		}

		/*
		 * Liberando o recurso se a macro PROTECT estiver definida
		 */

#ifdef PROTECT
		if( semop( g_sem_id_print, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif

#ifdef PROTECT
		if( semop( g_sem_id_cons, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif
	}
}

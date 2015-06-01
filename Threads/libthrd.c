/* Ce fichier contient des fonctions de thread */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#include "libthrd.h"

/* Global variables */
static int livingThreads = 0;
static int semid = -1;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};


static void sem_alloc(int nb) {
	#ifdef DEBUG
		fprintf(stderr, "Creating %d semaphores\n", nb);
	#endif
	semid = semget(IPC_PRIVATE, nb, 0666 | IPC_CREAT);
	if (semid < 0) { perror("libthrd.sem_alloc.semget failed"); exit(EXIT_FAILURE); }
}


void sem_free() {
	#ifdef DEBUG
		fprintf(stderr, "Freeing the semaphore\n");
	#endif
	int status = semctl (semid, 1, IPC_RMID, NULL);
	if (status < 0) perror ("libthrd.sem_free.semctl failed");
}

/* Initiates the semaphore at value val */
static void sem_init(int nb, unsigned short val) {
	int status = -1;
	union semun argument;
	unsigned short values[nb];

	/* Initializing semaphore values to val */
	memset(values, val, nb * sizeof(unsigned short));
	argument.array = values;

	status = semctl (semid, 0, SETALL, argument);
	if (status < 0) { perror("libthrd.sem_init.semctl failed"); exit(EXIT_FAILURE); }
}


static int PV(int index, int act) {
	struct sembuf op;
	op.sem_num = index;
	op.sem_op = act; /* P = -1; V = 1 */
	op.sem_flg = 0;

	return semop(semid, &op, 1);
}


void initMutexes(int nb, unsigned short val) {
	sem_alloc(nb);
	sem_init(nb, val);
}


void P(int index) {
	if (PV(index, -1) < 0) perror ("libthrd.P");
}


void V(int index) {
	if (PV(index, 1) < 0) perror ("libthrd.V");
}


void* lanceFunction(void *arg) {
	/* Copie de l'argument */
	Parameters *funcParameters = arg;
	/* Appel de la fonction avec l'argument dans la structure */
	funcParameters->fonction(funcParameters->argument);
	/* Liberation de la memoire */
	free(funcParameters->argument);
	free(funcParameters);

	livingThreads--;
	#ifdef DEBUG
		fprintf(stderr, "Thread terminated, total remaining: %d\n", livingThreads);
	#endif

	pthread_exit(NULL);
}


/* Returns 0 on success, negative integer if failed */
int lanceThread(void (*func)(void *), void *arg, int size) {
	if (livingThreads == MAX_THREADS) {
		#ifdef DEBUG
			perror("lanceThread.thread_limit");
		#endif
		return -5;
	}

	Parameters* funcParameters = (Parameters*) malloc(sizeof(Parameters));
	if (funcParameters == NULL) {
		#ifdef DEBUG
			perror("lanceThread.funcParameters.malloc");
		#endif
		return -1;
	}
	funcParameters->fonction = func;
	funcParameters->argument = malloc(size);
	if (funcParameters->argument == NULL) {
		#ifdef DEBUG
			perror("lanceThread.funcParameters.argument.malloc");
		#endif
		return -2;
	}
	memcpy(funcParameters->argument, arg, (size_t)size);

	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&tid, &attr, lanceFunction, funcParameters) != 0) {
		#ifdef DEBUG
			perror("lanceThread.pthread_create");
		#endif
		return -3;
	}

	livingThreads++;
	#ifdef DEBUG
		fprintf(stderr, "Thread started, total running: %d\n", livingThreads);
	#endif
	return 0;
}


int getLivingThreads() {
	return livingThreads;
}

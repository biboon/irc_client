#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <libcom.h>
#include <libthrd.h>

#define MAXLENSRV 50
#define TIMEOUT 10

static bool _quit = false;


void startSendLoop(void* arg) {
	#ifdef DEBUG
		fprintf(stdout, "Starting send loop\n");
	#endif
	int sock = *((int*) arg);
	clientSendLoop(sock);
}

void startReceiveLoop(void* arg) {
	#ifdef DEBUG
		fprintf(stdout, "Starting receive loop\n");
	#endif
	int sock = *((int*) arg);
	clientReceiveLoop(sock);
}


int main(int argc, char** argv) {
	/* Analyzing options */
	int option = 0;
	char* port = "6667";
	char* server = (char *) malloc(MAXLENSRV * sizeof(char));
	sprintf(server, "irc.rizon.net");

	/* Getting options */
	while ((option = getopt(argc, argv, "p:s:")) != -1) {
		switch (option) {
			case 'p':
				port = optarg;
				break;
			case 's':
				server = optarg;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option -%c\n", optopt);
				return -1;
			default:
				fprintf(stderr, "argument error\n");
				return -1;
		}
	}

	printf("Trying to connect to %s/%s\n", server, port);
	int sock = connexionServeur(server, port);
	if (sock < 0) perror("Could not connect");
	else printf("Connected to server sock #%d\n", sock);

	/* Starting threads */
	if (lanceThread(&startReceiveLoop, (void *) &sock, sizeof(int)) < 0)
		perror("Could not start receive loop");
	if (lanceThread(&startSendLoop, (void *) &sock, sizeof(int)) < 0)
		perror ("Could not start send loop");

	/* Let working... */
	while (!_quit) sleep(1);
	
	/* Waiting for threads to end */
	int time = 0;
	while (getLivingThreads() > 0 && time < TIMEOUT) { sleep(1); time++; }
	if (time == TIMEOUT) printf("Client quit timeout\n");

	return 0;
}

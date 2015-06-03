#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <libcom.h>
#include <libirc.h>

#define STRLEN 30
#define TIMEOUT 10


void usage(char* pgm) {
	printf("Usage: %s -n nickname -u username [-s server] [-p port]\n", pgm);
	printf("Default server: irc.rizon.net\n");
	printf("Default port: 6667\n");
}


int main(int argc, char** argv) {
	/* Analyzing options */
	int option = 0;
	char* port = (char *) malloc(5 * sizeof(char));
	char* server = (char *) malloc(STRLEN * sizeof(char));
	char* nick = (char *) malloc(STRLEN * sizeof(char));
	char* usr = (char *) malloc(STRLEN * sizeof(char));
	bool fnick = false, fusr = false;
	/* Setting default values */
	sprintf(server, "irc.rizon.net");
	sprintf(port, "6667");

	/* Getting options */
	while ((option = getopt(argc, argv, "p:s:n:u:")) != -1) {
		switch (option) {
			case 'p':
				port = optarg;
				break;
			case 's':
				server = optarg;
				break;
			case 'n':
				nick = optarg;
				fnick = true;
				break;
			case 'u':
				usr = optarg;
				fusr = true;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option -%c\n", optopt);
				usage(argv[0]);
				return -1;
			default:
				fprintf(stderr, "Argument error\n");
				usage(argv[0]);
				return -1;
		}
	}

	/* Checking nickname and username has been set */
	if (!fnick || !fusr) { usage(argv[0]); return -1; }

	printf("Trying to connect to %s:%s\n", server, port);
	int sock = connexionServeur(server, port);
	if (sock < 0) { perror("Could not connect"); return -1; }
	else printf("Connected to server sock #%d\n", sock);

	/* Setting user info */
	if (setupUser(sock, nick, usr) < 0) return -1;

	/* Starting main loop */
	clientLoop(sock, 0, procIncomingMessage, procOutgoingMessage);

	return 0;
}

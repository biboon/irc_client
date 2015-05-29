#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFSIZE 4096


static bool _quit = false;

void stopConnexions() {
	_quit = true;
}


/* Hote port */
int connexionServeur(char *hote, char *service){
	struct addrinfo precisions, *resultat, *origine;
	int statut, s;

	/* Creation de l'adresse de socket */
	memset(&precisions, 0, sizeof precisions);
	precisions.ai_family = AF_UNSPEC;
	precisions.ai_socktype = SOCK_STREAM;
	statut = getaddrinfo(hote, service, &precisions, &origine);
	if (statut < 0) { perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }
	struct addrinfo *p;
	for (p = origine, resultat = origine; p != NULL; p = p->ai_next)
		if (p->ai_family == AF_INET6) { resultat = p; break; }

	/* Creation d'une socket */
	s = socket(resultat->ai_family, resultat->ai_socktype, resultat->ai_protocol);
	if (s < 0) { perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

	/* Connection de la socket a l'hote */
	if (connect(s, resultat->ai_addr, resultat->ai_addrlen) < 0) return -1;

	/* Liberation de la structure d'informations */
	freeaddrinfo(origine);

	return s;
}

void clientReceiveLoop(int sock) {
	char buf[BUFSIZE];
	int length;
	while (!_quit) {
		length = read(sock, buf, BUFSIZE);
		if (length <= 0) break;
		fprintf(stdout, "Server says: %s", buf);
	}
}

void clientSendLoop(int sock) {
	char buf[BUFSIZE];
	int length;
	while (!_quit) {
		length = read(0, buf, BUFSIZE);
		if (length <= 0) break;
		write(sock, buf, length);
	}
}

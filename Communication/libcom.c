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
	struct addrinfo precisions, *resultat, *p;
	int statut, s;

	/* Creation de l'adresse de socket */
	memset(&precisions, 0, sizeof precisions);
	precisions.ai_family = AF_UNSPEC;
	precisions.ai_socktype = SOCK_STREAM;
	statut = getaddrinfo(hote, service, &precisions, &resultat);
	if (statut < 0) { perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }

	for (p = resultat; p != NULL; p = p->ai_next) {
		s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (s < 0) continue;
		if (connect(s, p->ai_addr, p->ai_addrlen) == 0) break; /*Success */
	}

	/* Liberation de la structure d'informations */
	freeaddrinfo(resultat);

	return s;
}

void clientReceiveLoop(int sock, void (*traitement)(char *, int)) {
	int length;
	while (!_quit) {
		char buf[BUFSIZE];
		length = read(sock, buf, BUFSIZE);
		if (length <= 0) { perror("libcom.clientReceiveLoop.read"); break; }
		traitement(buf, length);
	}
}

void clientSendLoop(int sock, void (*traitement)(int, char *, int)) {
	int length;
	while (!_quit) {
		char buf[BUFSIZE];
		length = read(0, buf, BUFSIZE);
		if (length < 0) { perror("libcom.clientSendLoop.read"); break; }
		traitement(sock, buf, length);
	}
}

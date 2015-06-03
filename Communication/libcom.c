#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#define BUFSIZE 2048


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

void clientLoop(int sock, int iface, void (*inProc)(int, char*, int), void (*outProc)(int, char*, int)) {
	/* Initializing structure */
	struct pollfd fds[2];
	memset(fds, -1, sizeof(fds));
	fds[0].fd = sock;
	fds[1].fd = iface;
	fds[0].events = POLLIN;
	fds[1].events = POLLIN;

	while (!_quit) {
		if (poll(fds, 2, 10) < 0) { perror("clientLoop.poll"); exit(EXIT_FAILURE); }

		int length = -1, i;

		for (i = 0; i < 2; i++) {
			if (fds[i].revents & POLLIN) { /* receiving data from server/iface */
				char *buf = (char*) malloc((BUFSIZE + 1) * sizeof(char));
				length = read(fds[i].fd, buf, BUFSIZE);
				if (length < 0) { perror("clientLoop.read"); free(buf); exit(EXIT_FAILURE); }
				buf[length] = '\0';

				if (i == 0) inProc(sock, buf, length);
				else outProc(sock, buf, length);
				
				free(buf);
			}
		}
	}
}

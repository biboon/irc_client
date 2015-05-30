#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"

#define BUFSIZE 4096

static char channel[20];
static int channelset = 0;


int setupUser(int sock, char* nick, char* usr) {
	char buf[BUFSIZE];
	int length;
	if (read(sock, buf, BUFSIZE) < 0) { perror("libcom.setUpUser.read"); return -1; }
	if (strstr(buf, "Found your hostname") != NULL) {
		length = sprintf(buf, "NICK %s\n", nick);
		if (length != write(sock, buf, length)) { perror("libcom.setupUser.write"); return -3; }
		length = sprintf(buf, "USER %s 8 * :%s\n", nick, usr);
		if (length != write(sock, buf, length)) { perror("libcom.setupUser.write"); return -4; }
		printf("Connected !\n");
		return 0;
	} else {
		printf("Could not connect to the server\n");
		return -2;
	}
}


void procIncomingMessage(char* msg, int size) {
	printf("Srv >> %s\n", msg);
}


void procOutgoingMessage(int sock, char* msg, int size) {
	int length, valid = 1;
	char* buf = (char*) malloc((size + 30) * sizeof(char));
	char* tmp = (char*) malloc(size * sizeof(char));
	*buf = '\0'; *tmp = '\0';
	strncat(tmp, msg, size);

	if (tmp[0] != '/') {
		if (channelset == 0) { /* Checking if the channel is set */
			printf("Can't send the message: join a channel first\n");
			return;
		} else {
			length = sprintf(buf, "PRIVMSG %s :%s\n", channel, tmp);
		}
	}
	else if (strstr(tmp, "join") == (tmp + 1)) {
		if (channelset == 1)
			printf("Already joined channel %s\n", channel);
		else if (sscanf(tmp, "/join %s\n", channel) == 1) {
			length = sprintf(buf, "JOIN %s\n", channel);
			printf("Joining channel %s\n", channel);
			channelset = 1;
		} else {
			printf("Error command: %s\n", tmp); valid = 0;
		}
	}
	else if (strstr(tmp, "msg") == (tmp + 1)) {
		char* dest = (char*) malloc(20 * sizeof(char));
		char* message = (char*) malloc(size * sizeof(char));
		if (sscanf(tmp, "/msg \"%s\" %s\n", dest, message) == 2)
			length = sprintf(buf, "PRIVMSG %s :%s\n", dest, message);
		else {
			printf("Error command: %s: %s %s\n", tmp); valid = 0;
		}
		free(dest); free(message);
	}
	else if (strstr(tmp, "quit") == (tmp + 1)) {
		char* message = (char*) malloc(size * sizeof(char));
		if (sscanf(tmp, "/quit %s\n", message) == 1)
			length = sprintf(buf, "QUIT :%s\n", message);
		else
			length = sprintf(buf, "QUIT\n");
		free(message);
	}
	else {
		printf("Invalid command \"%s\"\n", buf); valid = 0;
	}

	if (valid == 1) {
		write(sock, buf, length);
		#ifdef DEBUG
			printf("\tCommand sent: >>%s<<\n", buf);
		#endif
	}
	free(buf); free(tmp);
}

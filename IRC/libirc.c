#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"

#define BUFSIZE 4096
#define NAMELEN 20
#define CMDLEN 20

static char channel[NAMELEN];
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
	char* tmp = (char*) malloc(size * sizeof(char));
	char* sender = (char*) malloc(NAMELEN * sizeof(char));
	char* dest = (char*) malloc(NAMELEN * sizeof(char));
	char* message = (char*) malloc(BUFSIZE * sizeof(char));

	*tmp = '\0';
	strncat(tmp, msg, size);

	if (getMsgReceived(tmp, sender, dest, message) == 3) {
		if (dest[0] == '#')
			printf("%12s | %s\n", sender, message);
		else
			printf(">%10s< | %s\n", sender, message);
	} else
		printf("Srv >> %s\n", tmp);
}


void procOutgoingMessage(int sock, char* msg, int size) {
	int length, valid = 1;
	char* buf = (char*) malloc((size + CMDLEN) * sizeof(char));
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
		char* dest = (char*) malloc(NAMELEN * sizeof(char));
		char* message = (char*) malloc(size * sizeof(char));
		if (setMsgDest(tmp, dest, message) == 2)
			length = sprintf(buf, "PRIVMSG %s :%s\n", dest, message);
		else {
			printf("Error command: %s\n", tmp); valid = 0;
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


/* Equivalent to reg expression \/msg [[:alphanum]] [[:alphanum:] ] */
int setMsgDest(char* buf, char* dest, char* msg) {
	int i = 0, j = 0, res = 0;
	char cmd[5] = "/msg ";
	for (i = 0; i < 5; i++) {
		if (cmd[i] != buf[i]) return -1;
	}

	while (buf[i] == ' ' && buf[i] != '\0') i++; /* ignoring spaces */

	while (buf[i] != ' ' && buf[i] != '\0') { /* getting dest name */
		dest[j] = buf[i];
		i++; j++;
	}
	dest[j] = '\0'; if (j != 0) res++;

	while (buf[i] == ' ' && buf[i] != '\0') i++; /* ignoring spaces */

	j = 0;
	while (buf[i] != '\0') { /* getting message */
		msg[j] = buf[i];
		i++; j++;
	}
	msg[j] = '\0'; if (j != 0) res++;

	return res;
}

int getMsgReceived(char* buf, char* sender, char* dest, char* msg) {
	int i = 0, j = 0, res = 0;
	char cmd[9] = " PRIVMSG "; cmd[9] = '\0';

	while (buf[i] != ':' && buf[i] != '\0') i++; /* Ignoring chars before ':' */
	i++;

	while (buf[i] != '!' && buf[i] != '\0') { /* Getting sender name */
		sender[j] = buf[i];
		i++; j++;
	}
	sender[j] = '\0'; if (j != 0) res++;

	while (buf[i] != ' ' && buf[i] != '\0') i++; /* Ignoring chars before command */

	j = 0;
	while (cmd[j] != '\0' && buf[i] != '\0') { /* Checking the PRIVMSG command */
		if (buf[i] != cmd[j])
			return -1;
		i++; j++;
	}

	j = 0;
	while (buf[i] != ' ' && buf[i] != '\0') { /*Getting the destination name */
		dest[j] = buf[i];
		i++; j++;
	}
	dest[j] = '\0'; if (j != 0) res++;

	while (buf[i] != ':' && buf[i] != '\0') i++; /* Ignoring chars before ':' */
	i++;

	j = 0;
	while (buf[i] != '\n' && buf[i] != '\0') { /* Getting the message */
		msg[j] = buf[i];
		i++; j++;
	}
	msg[j] = '\0'; if (j != 0) res++;

	return res;
}

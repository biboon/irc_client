#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"

#define BUFSIZE 2048
#define NAMELEN 40
#define CMDLEN 20

static char channel[NAMELEN];
static int channelset = 0;


int setupUser(int sock, char* nick, char* usr) {
	char* buf = (char*) malloc((BUFSIZE + 1) * sizeof(char)); buf[0] = '\0';
	int length;

	if (read(sock, buf, BUFSIZE) < 0) {
		perror("libcom.setUpUser.read");
		free(buf);
		return -1;
	}

	if (strstr(buf, "Found your hostname") != NULL) {
		length = sprintf(buf, "NICK %s\n", nick);
		if (length != write(sock, buf, length)) {
			perror("libcom.setupUser.write");
			free(buf);
			return -3;
		}
		#ifdef DEBUG
			printf("Command sent: %s\n", buf);
		#endif

		length = sprintf(buf, "USER %s 8 * :%s\n", nick, usr);
		if (length != write(sock, buf, length)) {
			perror("libcom.setupUser.write");
			free(buf);
			return -4;
		}
		#ifdef DEBUG
			printf("Command sent: %s\n", buf);
		#endif

		printf("Connected !\n");
		free(buf);
		return 0;
	} else {
		printf("Could not connect to the server\n");
		free(buf);
		return -2;
	}
}


void procIncomingMessage(int sock, char* msg, int size) {
	char* sender = (char*) malloc((NAMELEN + 1) * sizeof(char));
	char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
	char* message = (char*) malloc(size * sizeof(char));

	if (getMsgReceived(msg, size, sender, dest, message) == 3) {
		printf(((dest[0] == '#') ? "%12s | %s\n" : ">%10s< | %s\n"), sender, message);
	} else if (getPingRequest(msg, size, sender) == 2) {
		#ifdef DEBUG
			printf("Sending PONG to %s\n", sender);
		#endif
		int length = sprintf(message, "PONG :%s\n", sender);
		write(sock, message, length);
	} else
		printf("Srv >> %s\n", msg);

	free(sender); free(dest); free(message);
}


void procOutgoingMessage(int sock, char* msg, int size) {
	int length, valid = 1;
	char* buf = (char*) malloc((size + NAMELEN + CMDLEN) * sizeof(char));

	if (msg[0] != '/') {
		if (channelset == 0) { /* Checking if the channel is set */
			printf("Can't send the message: join a channel first\n");
			return;
		} else {
			length = sprintf(buf, "PRIVMSG %s :%s", channel, msg);
		}
	}
	else if (strstr(msg, "join") == (msg + 1)) {
		if (channelset == 1)
			printf("Already joined channel %s\n", channel);
		else if (sscanf(msg, "/join %s\n", channel) == 1) {
			length = sprintf(buf, "JOIN %s\n", channel);
			printf("Joining channel %s\n", channel);
			channelset = 1;
		} else {
			printf("Error command: %s\n", msg); valid = 0;
		}
	}
	else if (strstr(msg, "msg") == (msg + 1)) {
		char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
		char* message = (char*) malloc(size * sizeof(char));
		if (setMsgDest(msg, dest, message) == 2)
			length = sprintf(buf, "PRIVMSG %s :%s\n", dest, message);
		else {
			printf("Error command: %s\n", msg); valid = 0;
		}
		free(dest); free(message);
	}
	else if (strstr(msg, "quit") == (msg + 1)) {
		char* message = (char*) malloc(size * sizeof(char));
		if (sscanf(msg, "/quit %s\n", message) == 1)
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
	free(buf);
}


/* Equivalent to reg expression \/msg [[:alphanum:]] [[:alphanum:] ] */
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

int getMsgReceived(char* buf, int size, char* sender, char* dest, char* msg) {
	int i = 0, j = 0, res = 0;
	char cmd[9] = " PRIVMSG "; cmd[9] = '\0';

	while (i < size && buf[i] != ':') i++; /* Ignoring chars before ':' */
	i++;

	while (i < size && buf[i] != '!' && j < NAMELEN) { /* Getting sender name */
		sender[j] = buf[i];
		i++; j++;
	}
	sender[j] = '\0'; if (j != 0) res++;

	while (i < size && buf[i] != ' ') i++; /* Ignoring chars before command */

	j = 0;
	while (i < size && cmd[j] != '\0') { /* Checking the PRIVMSG command */
		if (buf[i] != cmd[j])
			return -1;
		i++; j++;
	}

	j = 0;
	while (i < size && buf[i] != ' ' && j < NAMELEN) { /*Getting the destination name */
		dest[j] = buf[i];
		i++; j++;
	}
	dest[j] = '\0'; if (j != 0) res++;

	while (i < size && buf[i] != ':') i++; /* Ignoring chars before ':' */
	i++;

	j = 0;
	while (i < size && buf[i] != '\n' && j < size - 1) { /* Getting the message */
		msg[j] = buf[i];
		i++; j++;
	}
	msg[j] = '\0'; if (j != 0) res++;

	return res;
}

int getPingRequest(char* buf, int size, char* sender) {
	int i = 0, j = 0, res = 0;
	char cmd[6] = "PING :"; cmd[6] = '\0';

	while (i < size && cmd[i] != '\0') { /* checking we got a ping request */
		if (cmd[i] != buf[i]) return -1;
		i++;
	}
	res++;

	while (i < size && buf[i] != '\n' && j < NAMELEN) { /* Getting the sender name */
		sender[j] = buf[i];
		i++; j++;
	}
	sender[j] = '\0'; if (j != 0) res++;
	return res;
}

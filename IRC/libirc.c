#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"
#include "colors.h"


#define BUFSIZE 4096
#define NAMELEN 40
#define CMDLEN 20

static char channel[NAMELEN];
static int channelset = 0;


int setupUser(int sock, char* nick, char* usr, int retries) {
	#ifdef DEBUG
		printf("setupUser: %d try left\n", retries);
	#endif

	if (retries == 0)
		return -1;

	char* buf = (char*) malloc((BUFSIZE + 1) * sizeof(char)); buf[0] = '\0';
	int length;

	length = read(sock, buf, BUFSIZE);

	if (length < 0) {
		perror("libcom.setUpUser.read");
		free(buf);
		return -2;
	}
	else {
		buf[length] = '\0';
		if (strstr(buf, "Found your hostname") != NULL) {
			#ifdef DEBUG
				setcolor(GREEN);
				printf("Srv >> %s", buf);
				setcolor(RESET);
				fflush(stdout);
			#endif

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
			setcolor(GREEN);
			printf("Srv >> %s", buf);
			setcolor(RESET);
			fflush(stdout);
			free(buf);

			return setupUser(sock, nick, usr, retries - 1);
		}
	}
}


int procIncomingMessage(int sock, char* msg, int size) {
	char* sender = (char*) malloc((NAMELEN + 1) * sizeof(char));
	char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
	char* message = NULL;

	#ifdef DEBUG
		printf("Received the message: \"%s\"\n", msg);
	#endif

	if (getMsgReceived(msg, size, sender, dest, &message) == 3) {
		if (dest[0] == '#')
			printf("%12s | %s", sender, message);
		else {
			setcolor(RED);
			printf("%12s", sender);
			setcolor(RESET);
			printf(" | %s", message);
		}
		fflush(stdout);
	} else if (getPingRequest(msg, size, sender) == 2) {
		#ifdef DEBUG
			printf("Sending PONG to %s\n", sender);
		#endif
		char* buf = (char*) malloc((size + CMDLEN) * sizeof(char));
		int length = sprintf(buf, "PONG :%s\n", sender);
		if (length != write(sock, buf, length)) {
			perror("procIncomingMessage.write"); return -1;
		}
		free(buf);
	} else {
		#ifdef DEBUG
			printf("No pattern detected\n");
		#endif
		setcolor(GREEN);
		printf("Srv >> %s", msg);
		setcolor(RESET);
		fflush(stdout);
	}

	free(sender); free(dest);

	return 0;
}


int procOutgoingMessage(int sock, char* msg, int size) {
	int length, res, valid = 1;
	char* buf = (char*) malloc((size + NAMELEN + CMDLEN) * sizeof(char));

	if (msg[0] != '/') {
		res = 0;
		if (channelset == 0) { /* Checking if the channel is set */
			printf("Can't send the message: join a channel first\n"); valid = 0;
		} else {
			length = sprintf(buf, "PRIVMSG %s :%s", channel, msg);
		}
	}
	else if (strstr(msg, "join") == (msg + 1)) {
		res = 1;
		if (channelset == 1) {
			printf("Already joined channel %s\n", channel);
			valid = 0;
		} else if (sscanf(msg, "/join %s\n", channel) == 1) {
			length = sprintf(buf, "JOIN %s\n", channel);
			printf("Joining channel %s\n", channel);
			channelset = 1;
		} else {
			printf("Error command: %s\n", msg); valid = 0;
		}
	}
	else if (strstr(msg, "msg") == (msg + 1)) {
		res = 2;
		char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
		char* message = NULL;
		if (setMsgDest(msg, dest, &message) == 2)
			length = sprintf(buf, "PRIVMSG %s :%s\n", dest, message);
		else {
			printf("Error command: %s\n", msg); valid = 0;
		}
		free(dest);
	}
	else if (strstr(msg, "leave") == (msg + 1)) {
		res = 3;
		if (channelset == 1) {
			length = sprintf(buf, "JOIN 0\n");
			channelset = 0;
			printf("Leaving channel %s\n", channel);
		} else { printf("No channel set\n"); valid = 0; }
	}
	else if (strstr(msg, "quit") == (msg + 1)) {
		res = -1;
		char* message = (char*) malloc(size * sizeof(char));
		if (sscanf("/quit %s\n", message) != 0)
			length = sprintf(buf, "QUIT :%s\n", message);
		else
			length = sprintf(buf, "QUIT\n");
		free(message);
		printf("Quitting\n");
	}
	else {
		res = 0;
		printf("Invalid command \"%s\"\n", msg); valid = 0;
	}

	if (valid == 1) {
		if (length != write(sock, buf, length)) {
			perror("procOutgoingMessage.write"); return -1;
		}
		#ifdef DEBUG
			printf("\tCommand sent: %s\n", buf);
		#endif
	}
	free(buf);

	return res;
}


/* Equivalent to reg expression \/msg [[:alphanum:]] [[:alphanum:] ] */
int setMsgDest(char* buf, char* dest, char** msg) {
	int i = 0, res = 0;
	char* cmd = (char*) malloc((CMDLEN + 1) * sizeof(char));

	res = sscanf(buf, "%s %s", cmd, dest);
	#ifdef DEBUG
		printf("setMsgDest got: res: %d cmd: %s dest: %s\n", res, cmd, dest);
	#endif

	if (res != 2 || strcmp("/msg", cmd) != 0)
		res = 0;
	else {
		res = 1; /* We already got the destination name */
		i = strstr(buf, dest) - buf;
		while (buf[i] != ' ' && buf[i] != '\0') i++; /* ignoring the dest name */
		while (buf[i] == ' ' && buf[i] != '\0') i++; /* ignoring spaces */

		*msg = buf + i;
		if (**msg != '\0') res++;

		#ifdef DEBUG
			printf("setMsgDest set: dest: %s msg: %s\n", dest, *msg);
		#endif
	}

	free(cmd);

	return res;
}


int getMsgReceived(char* buf, int size, char* sender, char* dest, char** msg) {
	int i = 0, j = 0, res = 0;
	char* cmd = (char*) malloc((CMDLEN + 1) * sizeof(char));
	char* longname = (char*) malloc(size * sizeof(char));

	res = sscanf(buf, "%s %s %s", longname, cmd, dest);
	#ifdef DEBUG
		printf("getMsgReceived got: res: %d longname: %s cmd: %s dest: %s\n", res, longname, cmd, dest);
	#endif

	if (res != 3 || strcmp("PRIVMSG", cmd) != 0)
		res = 0;
	else {
		res = 1; /* We already got the dest name */

		/* Ignoring chars before ':' */
		while (buf[i] != ':' && buf[i] != '\0') i++;
		i++;

		/* Getting sender name */
		while (longname[i + j] != '!' && longname[i + j] != '\0' && j < NAMELEN) {
			sender[j] = longname[i + j];
			j++;
		}
		if (j != 0) res++;
		sender[j] = '\0';
		i += j; j = 0;

		i = strstr(buf, dest) - buf;

		/* Ignoring until we get to the message */
		while (buf[i] != ':' && buf[i] != '\0') i++;
		if (buf[i] == ':') { i++; res++; }
		*msg = (buf + i);

		#ifdef DEBUG
			printf("getMsgReceived set: sender: %s dest: %s msg: %s\n", sender, dest, *msg);
		#endif
	}

	free(cmd); free(longname);

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"
#include "colors.h"
#include "codes.h"


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


/* Returns a code depending on the command entered */
/* The param pointer is set at the first byte of parameter if exists */
int getClientCmd(char* buf, char** param) {
	int i = 0, res = 0;

	/* Check the cmd starts with a / char */
	if (buf[0] != '/') {
		*param = buf;
		return SNOTCMD;
	} else {
		/* Getting the command */
		if (strstr(buf, "msg") == (buf + 1)) res = SMSG;
		else if (strstr(buf, "join") == (buf + 1)) res = SJOIN;
		else if (strstr(buf, "quit") == (buf + 1)) res = SQUIT;
		else if (strstr(buf, "leave") == (buf + 1)) res = SLEAVE;
	}

	while (buf[i] != ' ' && buf[i] != '\0') i++;
	while (buf[i] == ' ' && buf[i] != '\0') i++;
	*param = buf + i;

	return res;
}


/* Returns a code depending on the type of data received */
/* param is set to the first char of the string coming after the command */
/* excluding the ':' char */
int getServerCmd (char* buf, char** param) {
	int res = CMDERR, i = 0;
	char* ptr;

	/* Check if the first char is ':' */
	if (buf[0] == ':') {
		if ((ptr = strstr(buf, "PRIVMSG")) != NULL) res = PRIVMSG;
		else if ((ptr = strstr(buf, "JOIN")) != NULL) res = JOIN;
		else if ((ptr = strstr(buf, "PART")) != NULL) res = PART;
		else if ((ptr = strstr(buf, "QUIT")) != NULL) res = QUIT;
		else if ((ptr = strstr(buf, "NICK")) != NULL) res = NICK;
	} else {
		if ((ptr = strstr(buf, "PING")) != NULL) res = PING;
	}
	/* Ignoring until we get to the parameter */
	if (ptr != NULL) {
		while (ptr[i] != ' ' && ptr[i] != '\0') i++;
		while ((ptr[i] == ':' || ptr[i] == ' ') && ptr[i] != '\0') i++;
		*param = ptr + i;
	}

	return res;
}


int procIncomingMessage(int sock, char* msg, int size) {
	char* param = NULL;

	#ifdef DEBUG
		printf("Received the message: \"%s\"\n", msg);
	#endif

	int res = getServerCmd(msg, &param);
	#ifdef DEBUG
		printf("Got the server command code #%d\n", res);
	#endif

	if (res == PRIVMSG) {
		char* sender = (char*) malloc((NAMELEN + 1) * sizeof(char));
		char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
		if (getMsgInfo(msg, size, sender, dest) == 2) {
			if (dest[0] == '#')
				printf("%15s | %s", sender, param);
			else {
				setcolor(RED);
				printf("%15s", sender);
				setcolor(RESET);
				printf(" | %s", param);
			}
			fflush(stdout);
		}
		free(sender); free(dest);
	} else if (res == PING) {
		#ifdef DEBUG
			printf("Sending PONG to %s", param);
		#endif
		char* buf = (char*) malloc((size + CMDLEN) * sizeof(char));
		int length = sprintf(buf, "PONG :%s", param);
		if (length != write(sock, buf, length)) {
			perror("procIncomingMessage.write"); return -1;
		}
		free(buf);
	} else if (res == JOIN || res == PART || res == QUIT || res == NICK) {
		char* name = (char*) malloc((NAMELEN + 1) * sizeof(char));
		if (getChannelActivitySender(msg, name) == 1) {
			setcolor(GREEN);
			printf("%15s", "Server");
			setcolor(RESET);
			if (res == JOIN)
				printf(" | %s joined %s", name, param);
			else if (res == PART)
				printf(" | %s has left %s", name, param);
			else if (res == QUIT)
				printf(" | %s has quit %s", name, param);
			else if (res == NICK)
				printf(" | %s is now known as %s", name, param);
			fflush(stdout);
		}
		free(name);
	} else {
		#ifdef DEBUG
			printf("No pattern detected\n");
		#endif
		setcolor(GREEN);
		printf("Srv >> %s", msg);
		setcolor(RESET);
		fflush(stdout);
	}

	return 0;
}


int procOutgoingMessage(int sock, char* msg, int size) {
	int length, res, valid = 1;
	char* buf = (char*) malloc((size + NAMELEN + CMDLEN) * sizeof(char));
	char* param = NULL;

	res = getClientCmd(msg, &param);
	#ifdef DEBUG
		printf("Got the client command code #%d\n", res);
	#endif

	if (res == SNOTCMD) {
		if (channelset == 0) { /* Checking if the channel is set */
			printf("Can't send the message: join a channel first\n"); valid = 0;
		} else {
			length = sprintf(buf, "PRIVMSG %s :%s", channel, msg);
		}
	} else if (res == SMSG) {
		char* dest = (char*) malloc((NAMELEN + 1) * sizeof(char));
		char* message = NULL;
		if (setMsgDest(param, dest, &message) == 2)
			length = sprintf(buf, "PRIVMSG %s :%s\n", dest, message);
		else {
			printf("Error command: %s\n", msg); valid = 0;
		}
		free(dest);
	} else if (res == SJOIN) {
		if (channelset == 1) {
			printf("Already joined channel %s\n", channel);
			valid = 0;
		} else {
			int i = 0; /* Just getting the channel name without \n */
			while (param[i] != '\n' && param[i] != '\0' && i < NAMELEN) {
				channel[i] = param[i];
				i++;
			}
			channel[i] = '\0';
			length = sprintf(buf, "JOIN %s\n", channel);
			printf("Joining channel %s\n", channel);
			channelset = 1;
		}
	} else if (res == SLEAVE) {
		if (channelset == 1) {
			length = sprintf(buf, "JOIN 0\n");
			channelset = 0;
			printf("Leaving channel %s\n", channel);
		} else {
			printf("No channel set\n"); valid = 0;
		}
	} else if (res == SQUIT) {
		if (param[0] != '\0') {
			char* message = (char*) malloc(size * sizeof(char));
			length = sprintf(buf, "QUIT :%s\n", message);
			free(message);
		} else
			length = sprintf(buf, "QUIT\n");
		printf("Quitting\n");
	} else {
		res = SCMDERR;
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
	int res = sscanf(buf, "%s", dest);

	if (res != 1) res = 0;
	else {
		int i = 0;
		while (buf[i] != ' ' && buf[i] != '\0') i++; /* ignoring the dest name */
		while (buf[i] == ' ' && buf[i] != '\0') i++; /* ignoring spaces */

		*msg = buf + i;
		if (**msg != '\0') {
			res++;
			#ifdef DEBUG
				printf("setMsgDest set: dest: %s msg: %s\n", dest, *msg);
			#endif
		}
	}

	return res;
}


/* Gets the sender, the destination and the message */
int getMsgInfo(char* buf, int size, char* sender, char* dest) {
	int res = 0;
	char* longname = (char*) malloc(size * sizeof(char));

	res = sscanf(buf, "%s %*s %s", longname, dest);
	#ifdef DEBUG
	#endif

	if (res != 2) res = 0;
	else {
		res = 1; /* We already got the dest name */
		res += getChannelActivitySender(longname, sender);
	}

	free(longname);

	#ifdef DEBUG
		printf("getMsgInfo set: res: %d sender: %s dest: %s\n", res, sender, dest);
	#endif

	return res;
}


/* Gives sender name on channel activity */
/* :sender_name![random chars] */
int getChannelActivitySender(char* buf, char* name) {
	int i = 0, j = 0, res = 0;

	/* Ignoring chars before ':' */
	while (buf[i] != ':' && buf[i] != '\0') i++;
	i++;

	/* Getting sender name */
	while (buf[i + j] != '!' && buf[i + j] != '\0' && j < NAMELEN) {
		name[j] = buf[i + j];
		j++;
	}
	if (j != 0) res++;
	name[j] = '\0';

	return res;
}

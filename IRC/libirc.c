#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libirc.h"

#define BUFSIZE 4096


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

#ifndef __LIBIRC_H__
#define __LIBIRC_H__

int setupUser(int sock, char* nick, char* usr);
void procIncomingMessage(char* msg, int size);
void procOutgoingMessage(int sock, char* msg, int size);
int getMsgDest(char* buf, char* dest, char* msg);

#endif

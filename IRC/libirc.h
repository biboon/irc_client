#ifndef __LIBIRC_H__
#define __LIBIRC_H__

int setupUser(int sock, char* nick, char* usr);
void procIncomingMessage(int sock, char* msg, int size);
void procOutgoingMessage(int sock, char* msg, int size);
int setMsgDest(char* buf, char* dest, char* msg);
int getMsgReceived(char* buf, int size, char* sender, char* dest, char* msg);
int getPingRequest(char* buf, int size, char* sender);

#endif

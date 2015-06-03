#ifndef __LIBCOM_H__
#define __LIBCOM_H__

int connexionServeur(char *hote, char *service);
void clientLoop(int sock, int iface, void (*inProc)(int, char*, int), void (*outProc)(int, char*, int));

int setupUser(int sock, char* nick, char* usr);

#endif

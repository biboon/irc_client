#ifndef __LIBCOM_H__
#define __LIBCOM_H__

int connexionServeur(char *hote, char *service);
void clientReceiveLoop(int sock, void (*traitement)(char *, int));
void clientSendLoop(int sock, void (*traitement)(int, char *, int));

int setupUser(int sock, char* nick, char* usr);

#endif

#ifndef __LIBCOM_H__
#define __LIBCOM_H__

int connexionServeur(char *hote, char *service);
void clientLoop(int sock, int iface, int (*inProc)(int, char*, int), int (*outProc)(int, char*, int));

#endif

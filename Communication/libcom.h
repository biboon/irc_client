#ifndef __LIBCOM_H__
#define __LIBCOM_H__

int connexionServeur(char *hote, char *service);
void clientReceiveLoop(int sock);
void clientSendLoop(int sock);

#endif

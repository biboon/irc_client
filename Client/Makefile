# Makefile pour generer le serveur

OBJS = client.o
LOCAL_LIBS  = -L ../Communication -L ../IRC
LOCAL_LIBS += -lcom -lirc
CFLAGS     += -I ../Communication -I ../IRC

# Cible generale
all: client

# La cible de nettoyage
clean:
	rm -f core *.o client http

# Les cibles pour l'executable
client: $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJS) $(LOCAL_LIBS) $(LIBS)

client.o: client.c

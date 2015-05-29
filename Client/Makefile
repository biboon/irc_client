# Makefile pour generer le serveur

OBJS = client.o
LIBS = -lpthread
LOCAL_LIBS  = -L ../Communication -L ../Threads
LOCAL_LIBS += -lcom -lthrd
CFLAGS     += -I ../Communication -I ../Threads

# Cible generale
all: client

# La cible de nettoyage
clean:
	rm -f core *.o client http

# Les cibles pour l'executable
client: $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJS) $(LOCAL_LIBS) $(LIBS)

client.o: client.c


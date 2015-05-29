# Makefile pour generer la bibliotheque de fonctions de communication
LIB = libcom

# Regle generale
all: $(LIB).a

# Les cibles pour la bibliotheque
$(LIB).o: $(LIB).c $(LIB).h
	$(CC) $(CFLAGS) -o $(LIB).o -c $(LIB).c

# Cible de creation de la bibliotheque
$(LIB).a: $(LIB).o
	ar -q $(LIB).a $(LIB).o
	rm -f $(LIB).o

# La cible de nettoyage
clean: 
	rm -f core *.o $(LIB).a

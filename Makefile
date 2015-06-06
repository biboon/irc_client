# Makefile général
# Constantes pour la compilation des programmes
export CC = gcc
export LD = gcc
export CLIB = ar cq
export CFLAGS = -Wall


# Constantes liees au projet
DIRS=Communication IRC Client

# La cible generale
release: $(patsubst %, _dir_%, $(DIRS))

all: CFLAGS += -g
all: $(patsubst %, _dir_%, $(DIRS))

$(patsubst %,_dir_%,$(DIRS)):
	cd $(patsubst _dir_%,%,$@) && make

# La cible de nettoyage
clean: $(patsubst %, _clean_%, $(DIRS))

$(patsubst %,_clean_%,$(DIRS)):
	cd $(patsubst _clean_%,%,$@) && make clean

debug: CFLAGS += -DDEBUG -g
debug: $(patsubst %, _dir_%, $(DIRS))

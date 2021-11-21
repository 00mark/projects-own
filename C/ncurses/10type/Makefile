CC = /usr/bin/cc
LIBS = -lncurses -pthread
FLAGS = -march=native -O3 -msse4
INSTALLDIR = $(HOME)/.10type

all: 
ifneq ("$(wildcard $(INSTALLDIR))","")
	$(error $(INSTALLDIR) exists!)
endif
	mkdir $(INSTALLDIR)
	@touch stats
	@cp config tmp
	sed -i "s:INSTALLDIR:$(INSTALLDIR):g" config
	cp -r wordlists stats config $(INSTALLDIR)/
	@mv tmp config
	$(CC) $(LIBS) $(FLAGS) 10type.c -o 10type 
	@echo to run from any folder run \"mv 10type /usr/local/bin/\" as root

update: 
	$(CC) $(LIBS) $(FLAGS) -o 10type 10type.c
	@echo to run from any folder run \"mv 10type /usr/local/bin/\" as root

clean: 
	rm -rf $(INSTALLDIR)/
	rm -f ./10type

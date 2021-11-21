cursbrowse: cursbrowse.o check_file.o util.o
	gcc -lX11 -lncurses -lpthread cursbrowse.o check_file.o util.o -o cursbrowse
cursbrowse.o: cursbrowse.c cursbrowse.h
	gcc -c -march=native -O3 -msse4 cursbrowse.c
check_file.o: check_file.c check_file.h
	gcc -c -march=native -O3 -msse4 check_file.c
util.o: util.c util.h
	gcc -c -march=native -O3 -msse4 util.c
clean:
	rm -f cursbrowse.o
	rm -f check_file.o
	rm -f util.o
	rm -f cursbrowse

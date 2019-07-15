cc = gcc
ccflags = -g -I. -pthread

all:	server

server: server.o db.o
	$(cc) ${ccflags} ${LDflags} server.o db.o \
		-o server

server.o:	server.c db.h
	$(cc) server.c -c ${ccflags}

db.o:	db.c db.h
	$(cc) db.c -c ${ccflags}

clean:
	/bin/rm -f *.o server

INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= process1.c process2.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	process1 process2

clean:
	-rm -f *.o *.d

distclean:
	-rm -f *.o *.d

process1: process1.o
	$(CC) -g $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

process2: process2.o
	$(CC) -g $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

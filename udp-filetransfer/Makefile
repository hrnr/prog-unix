# my generic makefile

# compiler and linker
CC = cc

# includes and libraries
INCS = -I. -I/usr/include
LIBS = 

# compiler flags
CFLAGS = -std=c11 -pedantic -Wall -Og -g ${INCS}
LDFLAGS = -g ${LIBS}

# source files
SRC = client.c server.c
OBJ = ${SRC:.c=.o}

all: options client server

options:
	@echo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

server: server.o
	@echo CC server.c
	@${CC} -o server.out server.o ${LDFLAGS}

client: client.o
	@echo CC client.c
	@${CC} -o client.out client.o ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f server client ${OBJ}

.PHONY: all options clean

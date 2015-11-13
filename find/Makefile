# my generic makefile

# compiler and linker
CC = cc

# includes and libraries
INCS = -I. -I/usr/include
LIBS =

# compiler flags
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = -g ${LIBS}

# source files
SRC = a.c
OBJ = ${SRC:.c=.o}

all: options main

options:
	@echo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

main: ${OBJ}
	@echo CC -o $@ $^
	@${CC} -o $@ $^ ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f surf ${OBJ} surf-${VERSION}.tar.gz

.PHONY: all options clean

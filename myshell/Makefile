# my generic makefile

# compiler and linker
CC = cc

# includes and libraries
INCS = -I. -I/usr/include
LIBS = -lreadline

# compiler flags
CFLAGS = -std=c99 -pedantic -Wall -Og -g ${INCS}
LDFLAGS = -g ${LIBS}

# source files
SRC = myshell.c parse.c
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
	@echo CC $^
	@${CC} $^ ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f surf ${OBJ} surf-${VERSION}.tar.gz

.PHONY: all options clean

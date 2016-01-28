# my generic makefile

# compiler and linker
CC = cc

# includes and libraries
INCS = -I. -I/usr/include
LIBS = -pthread

# compiler flags
CFLAGS = -std=c11 -pthread -pedantic -Wall -Og -g ${INCS}
LDFLAGS = -g ${LIBS}

# source files
SRC = workers.c
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
	@rm -f a.out ${OBJ}

.PHONY: all options clean

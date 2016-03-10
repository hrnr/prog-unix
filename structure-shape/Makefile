
PROG=shapeup

TEST_FLAGS = -Og -fsanitize=undefined -fsanitize=address -lcmocka -DTEST

all: $(PROG) test

$(PROG): struct.c
	${CC} -o $(PROG) $?

test: struct.c test.c
	${CC} -o $(PROG)_tests struct.c test.c ${TEST_FLAGS}

clean:
	rm -f *.o $(PROG) $(PROG)_tests core a.out

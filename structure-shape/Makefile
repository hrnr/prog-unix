
PROG=shapeup

TEST_FLAGS = -Og -fsanitize=undefined -fsanitize=address -lcmocka -DTEST

all: $(PROG) test

$(PROG): struct.c
	gcc -o $(PROG) $?

test: struct.c test.c
	gcc -o $(PROG)_tests struct.c test.c ${TEST_FLAGS}

clean:
	rm -f *.o $(PROG) core a.out

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define	init_size 10

typedef struct {
	size_t count;
	const char *word;
} Counter;

static Counter *counter_arr;
static size_t counter_arr_size = init_size;

static void add_word(char *word) {
	for (Counter* it = counter_arr; it < counter_arr + counter_arr_size; ++it) {
		if (it->word == NULL) {
			it->word = word;
			it->count = 1;
			return;
		}
		if (strcmp(word, it->word) == 0) {
			++it->count;
			free(word);
			return;
		}
	}
	// array is too small
	counter_arr = realloc(counter_arr, sizeof (Counter)*counter_arr_size*2); // double array size
	memset(counter_arr + counter_arr_size, 0, sizeof (Counter)*counter_arr_size); // zero new part of array
	counter_arr_size *= 2;
	// try again with larger array
	add_word(word);
}

int main(int argc, char const *argv[]) {
	// allocate array for counting
	counter_arr = calloc(init_size, sizeof (Counter));

	char *word = NULL;
	while (scanf("%ms", &word) > 0) {
		// %m is posix 2008 extension od ISO C
		add_word(word);
	}
	for (Counter* it = counter_arr; it < counter_arr + counter_arr_size; ++it) {
		if (it->word == NULL)
			return (0);

		printf("%s %lu\n", it->word, it->count);
	}
	return (0);
}
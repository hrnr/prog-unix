#include "f.h"

#include <stdarg.h>
#include <limits.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

enum op { MIN, MAX };
int min_max(enum op op, size_t n, va_list list);

int min_max(enum op op, size_t n, va_list list) {
	int current_val;

	if (op == MIN) {
		current_val = INT_MAX;
	} else {
		current_val = INT_MIN;
	}

	for (size_t i = 0; i < n; ++i) {
		if (op == MIN) {
			current_val = MIN(current_val, va_arg(list, int));
		} else {
			current_val = MAX(current_val, va_arg(list, int));
		} 
	}

	return current_val;
}

int f_max(size_t n, ...) {
	int max;

	va_list arglist;
	va_start(arglist, n);
	max = min_max(MAX, n, arglist);
	va_end(arglist);
	return (max);
}

int f_min(size_t n, ...) {
	int min;

	va_list arglist;
	va_start(arglist, n);
	min = min_max(MIN, n, arglist);
	va_end(arglist);
	return (min);
}

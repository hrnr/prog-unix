#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <string.h>
#include "../vec/vec.h"

/* protocol: size of string followed by string itself */

static inline void write_string(int fd, const char* string) {
	if(!string) {
		return;
	}

	size_t len = strlen(string) + 1; // incl. null byte
	write(fd, &len, sizeof(size_t));
    write(fd, string, len);
}

static inline int read_string(int fd, Vec *buffer) {
	size_t size;
	if(read(fd, &size, sizeof(size_t)) > 0) {
		vec_resize(buffer, size);
        read(fd, vec_begin(buffer), size);
        return 0;
	} else {
		return -1;
	}

}

#endif
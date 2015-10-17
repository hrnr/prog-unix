#ifndef VEC_H_
#define VEC_H_

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define _MIN(a,b) (((a)<(b))?(a):(b))
#define _MAX(a,b) (((a)>(b))?(a):(b))

typedef struct
{
	char *_begin;
	char *_end;
	char *_end_of_storage;
	size_t _elem_size;
} Vec;

static inline Vec* vec_init(size_t elem_size);
static inline void vec_free(Vec *vec);
static inline void* vec_at(Vec *vec, size_t index);
static inline void* vec_back(Vec *vec);
static inline void* vec_begin(Vec *vec);
static inline void* vec_end(Vec *vec);
static inline _Bool vec_empty(Vec *vec);
static inline size_t vec_size(Vec *vec);
static inline void vec_reserve(Vec *vec, size_t new_cap);
static inline size_t vec_capacity(Vec *vec);
static inline void vec_shrink_to_fit(Vec *vec);
static inline void vec_clear(Vec *vec);
static inline void* vec_insert(Vec *vec, void *position);
static inline void vec_insert_copy(Vec *vec, void *position, void *value);
static inline void* vec_insert_multi(Vec *vec, void *position, size_t count);
static inline void vec_insert_multi_copy(Vec *vec, void *position, size_t count, void *data);
static inline void* vec_push(Vec *vec);
static inline void vec_push_copy(Vec *vec, void* value);
static inline void* vec_push_multi(Vec *vec, size_t count);
static inline void vec_push_multi_copy(Vec *vec, size_t count, void *data);
static inline void vec_resize(Vec *vec, size_t count);
/* TODO erase, pop_back */
static inline void _vec_reserve(Vec *vec, size_t new_cap);
static inline void _vec_add_space(Vec *vec, size_t count);

static inline Vec* vec_init(size_t elem_size) {
	Vec *vec = malloc(sizeof (Vec));
	vec->_begin = NULL;
	vec->_end = NULL;
	vec->_end_of_storage = NULL;
	vec->_elem_size = elem_size;

	return vec;
}

static inline void vec_free(Vec *vec) {
	free(vec->_begin);
	free(vec);
}

static inline void* vec_at(Vec *vec, size_t index) {
	return vec->_begin + (index * vec->_elem_size);
}

static inline void* vec_back(Vec *vec) {
	return vec->_end - vec->_elem_size;
}

static inline void* vec_begin(Vec *vec) {
	return vec->_begin;
}

static inline void* vec_end(Vec *vec) {
	return vec->_end;
}

static inline _Bool vec_empty(Vec *vec) {
	return vec->_begin == vec->_end;
}

static inline size_t vec_size(Vec *vec) {
	return (size_t)(vec->_end - vec->_begin)/vec->_elem_size;
}

static inline void vec_reserve(Vec *vec, size_t new_cap) {
	if(new_cap <= vec_capacity(vec))
		return;
	
	_vec_reserve(vec, new_cap);
}

static inline size_t vec_capacity(Vec *vec) {
	return (size_t)(vec->_end_of_storage - vec->_begin)/vec->_elem_size;
}

static inline void vec_shrink_to_fit(Vec *vec) {
	_vec_reserve(vec, vec_size(vec));
}

static inline void vec_clear(Vec *vec) {
	vec->_end = vec->_begin;
}

static inline void* vec_insert(Vec *vec, void *position) {
	return vec_insert_multi(vec, position, 1);
}

static inline void vec_insert_copy(Vec *vec, void *position, void *value) {
	vec_insert_multi_copy(vec, position, 1, value);
}

static inline void* vec_insert_multi(Vec *vec, void *position, size_t count) {
	ptrdiff_t offset = (char*)position - vec->_begin;
	_vec_add_space(vec, count); /* pointer to pos may be invalidated here */
	char *new_elem = vec->_begin + offset;
	memmove(new_elem + vec->_elem_size*count, new_elem, vec->_end - new_elem);
	vec->_end = vec->_end + vec->_elem_size*count;
	return new_elem;
}

static inline void vec_insert_multi_copy(Vec *vec, void *position, size_t count, void *data) {
	void *new_elem_start = vec_insert_multi(vec, position, count);
	memcpy(new_elem_start, data, vec->_elem_size*count);
}

static inline void* vec_push(Vec *vec) {
	return vec_push_multi(vec, 1);
}

static inline void vec_push_copy(Vec *vec, void* value) {
	vec_push_multi_copy(vec, 1, value);
}

/* not using vec_insert... to always avoid memmove */
static inline void* vec_push_multi(Vec *vec, size_t count) {
	_vec_add_space(vec, count);
	void *new_elem_start = vec->_end;
	vec->_end = vec->_end + vec->_elem_size*count;
	return new_elem_start;
}

static inline void vec_push_multi_copy(Vec *vec, size_t count, void *data) {
	void *new_elem_start = vec_push_multi(vec, count);
	memcpy(new_elem_start, data, vec->_elem_size*count);
}

static inline void vec_resize(Vec *vec, size_t count) {
	_vec_add_space(vec, count);
	vec->_end = vec->_begin + vec->_elem_size*count;
}

/* like vec_reserve, but allows to reduce size */
static inline void _vec_reserve(Vec *vec, size_t new_cap) {
	size_t size = vec->_elem_size * vec_size(vec);
	size_t new_size = vec->_elem_size * new_cap;

	vec->_begin = realloc(vec->_begin, new_size);
	vec->_end = vec->_begin + _MIN(size, new_size);
	vec->_end_of_storage = vec->_begin + new_size;
}

static inline void _vec_add_space(Vec *vec, size_t count) {
	size_t size = vec_size(vec);
	if (size + count <= vec_capacity(vec))
		return; /* no need to alloc anything */

	/* we want to make space, and 2*0=0 */ 
	_vec_reserve(vec, size ? _MAX(2*size, count) : 1);
}

#endif /* VEC_H_ */
#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>

#define GROW_ARRAY(arr, sz, onfail)	\
	do { \
		int newsz = (sz) ? (sz) * 2 : 16; \
		void *tmp = realloc(arr, newsz * sizeof *(arr)); \
		if(!tmp) { \
			fprintf(stderr, "failed to grow array to %d\n", newsz); \
			onfail; \
		} \
		arr = tmp; \
		sz = newsz; \
	} while(0)

#define malloc_nf(sz)	malloc_nf_impl(sz, __FILE__, __LINE__)
void *malloc_nf_impl(size_t sz, const char *file, int line);

#define calloc_nf(num, sz)	calloc_nf_impl(num, sz, __FILE__, __LINE__)
void *calloc_nf_impl(size_t num, size_t sz, const char *file, int line);

#define realloc_nf(p, sz)	realloc_nf_impl(p, sz, __FILE__, __LINE__)
void *realloc_nf_impl(void *p, size_t sz, const char *file, int line);

#define strdup_nf(s)	strdup_nf_impl(s, __FILE__, __LINE__)
char *strdup_nf_impl(const char *s, const char *file, int line);

#endif	/* UTIL_H_ */

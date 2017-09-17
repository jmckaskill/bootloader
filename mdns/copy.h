#pragma once
#include <stddef.h>

static inline void copy_unaligned(void *dst, const void *src, size_t sz) {
	char *to = (char*) dst;
	char *from = (char*) src;
	while (sz) {
		*(to++) = *(from++);
		sz--;
	}
}


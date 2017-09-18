#pragma once
#include <stdint.h>

static inline void copy_unaligned(void *dst, const void *src, int sz) {
	char *to = (char*) dst;
	char *from = (char*) src;
	while (sz) {
		*(to++) = *(from++);
		sz--;
	}
}

#ifdef __arm__
#define ALIGNED_32 __attribute__((aligned (4)))
#else
#define ALIGNED_32
#endif

static inline void copy_aligned_32(void *dst, const void *src, int sz) {
	uint32_t *to = (uint32_t*) dst;
	uint32_t *from = (uint32_t*) src;
	while (sz > 0) {
		*(to++) = *(from++);
		sz -= 4;
	}
}


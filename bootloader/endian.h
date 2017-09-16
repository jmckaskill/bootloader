#pragma once

#include <stdint.h>

typedef struct {
#ifdef __BIG_ENDIAN__
    uint64_t high;
    uint64_t low;
#else
    uint64_t low;
    uint64_t high;
#endif
} uint128_t;

#ifdef __BIG_ENDIAN__
#define UINT128_C(HI, LO) {UINT64_C(HI), UINT64_C(LO)}
#else
#define UINT128_C(HI, LO) {UINT64_C(LO), UINT64_C(HI)}
#endif

static inline uint16_t read_little_16(const uint8_t *p) {
	return ((uint16_t) p[0])
		| ((uint16_t) p[1] << 8);
}
static inline uint32_t read_little_32(const uint8_t *p) {
	return ((uint32_t) p[0])
		| ((uint32_t) p[1] << 8)
		| ((uint32_t) p[2] << 16)
		| ((uint32_t) p[3] << 24);
}
static inline uint64_t read_little_48(const uint8_t *p) {
	return ((uint64_t) p[0])
		 | ((uint64_t) p[1] << 8)
		 | ((uint64_t) p[2] << 16)
		 | ((uint64_t) p[3] << 24)
		 | ((uint64_t) p[4] << 32)
		 | ((uint64_t) p[5] << 40);
}
static inline uint64_t read_little_64(const uint8_t *p) {
	return ((uint64_t) p[0])
		| ((uint64_t) p[1] << 8)
		| ((uint64_t) p[2] << 16)
		| ((uint64_t) p[3] << 24)
		| ((uint64_t) p[4] << 32)
		| ((uint64_t) p[5] << 40)
		| ((uint64_t) p[6] << 48)
		| ((uint64_t) p[7] << 56);
}
static inline uint128_t read_little_128(const uint8_t *p) {
    uint128_t ret;
    ret.low = read_little_64(p);
    ret.high = read_little_64(p+8);
    return ret;
}

static inline void write_little_16(uint8_t *p, uint16_t v) {
	p[0] = (uint8_t) (v);
	p[1] = (uint8_t) (v >> 8);
}
static inline void write_little_32(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t) (v);
	p[1] = (uint8_t) (v >> 8);
	p[2] = (uint8_t) (v >> 16);
	p[3] = (uint8_t) (v >> 24);
}
static inline void write_little_48(uint8_t *p, uint64_t v) {
	p[0] = (uint8_t) (v);
	p[1] = (uint8_t) (v >> 8);
	p[2] = (uint8_t) (v >> 16);
	p[3] = (uint8_t) (v >> 24);
	p[4] = (uint8_t) (v >> 32);
	p[5] = (uint8_t) (v >> 40);
}
static inline void write_little_64(uint8_t *p, uint64_t v) {
	p[0] = (uint8_t) (v);
	p[1] = (uint8_t) (v >> 8);
	p[2] = (uint8_t) (v >> 16);
	p[3] = (uint8_t) (v >> 24);
	p[4] = (uint8_t) (v >> 32);
	p[5] = (uint8_t) (v >> 40);
	p[6] = (uint8_t) (v >> 48);
	p[7] = (uint8_t) (v >> 56);
}
static inline void write_little_128(uint8_t *p, uint128_t v) {
    write_little_64(p, v.low);
    write_little_64(p+8, v.high);
}

static inline uint16_t read_big_16(const uint8_t *p) {
    return ((uint16_t) p[0] << 8) 
         | ((uint16_t) p[1]);
}
static inline uint32_t read_big_32(const uint8_t *p) {
    return ((uint32_t) p[0] << 24)
         | ((uint32_t) p[1] << 16)
         | ((uint32_t) p[2] << 8)
         | ((uint32_t) p[3]);
}
static inline uint64_t read_big_48(const uint8_t *p) {
    return ((uint64_t) p[0] << 40)
         | ((uint64_t) p[1] << 32)
         | ((uint64_t) p[2] << 24)
         | ((uint64_t) p[3] << 16)
         | ((uint64_t) p[4] << 8)
         | ((uint64_t) p[5]);
}
static inline uint64_t read_big_64(const uint8_t *p) {
    return ((uint64_t) p[0] << 56)
         | ((uint64_t) p[1] << 48)
         | ((uint64_t) p[2] << 40)
         | ((uint64_t) p[3] << 32)
         | ((uint64_t) p[4] << 24)
         | ((uint64_t) p[5] << 16)
         | ((uint64_t) p[6] << 8)
         | ((uint64_t) p[7]);
}
static inline uint128_t read_big_128(const uint8_t *p) {
    uint128_t ret;
    ret.high = read_big_64(p);
    ret.low = read_big_64(p+8);
    return ret;
}
static inline void write_big_16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t) (v >> 8);
    p[1] = (uint8_t) (v);
}
static inline void write_big_32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t) (v >> 24);
    p[1] = (uint8_t) (v >> 16);
    p[2] = (uint8_t) (v >> 8);
    p[3] = (uint8_t) (v);
}
static inline void write_big_48(uint8_t *p, uint64_t v) {
    p[0] = (uint8_t) (v >> 40);
    p[1] = (uint8_t) (v >> 32);
    p[2] = (uint8_t) (v >> 24);
    p[3] = (uint8_t) (v >> 16);
    p[4] = (uint8_t) (v >> 8);
    p[5] = (uint8_t) (v);
}
static inline void write_big_64(uint8_t *p, uint64_t v) {
    p[0] = (uint8_t) (v >> 56);
    p[1] = (uint8_t) (v >> 48);
    p[2] = (uint8_t) (v >> 40);
    p[3] = (uint8_t) (v >> 32);
    p[4] = (uint8_t) (v >> 24);
    p[5] = (uint8_t) (v >> 16);
    p[6] = (uint8_t) (v >> 8);
    p[7] = (uint8_t) (v);
}
static inline void write_big_128(uint8_t *p, uint128_t v) {
    write_big_64(p, v.high);
    write_big_64(p+8, v.low);
}

#if 0
static inline uint16_t hi16(uint32_t v) {
    return (uint16_t) (v >> 16);
}
static inline uint16_t lo16(uint32_t v) {
    return (uint16_t) (v & 0xFFFF);
}

static inline uint16_t ntohs(uint16_t v) {
    union {
        uint16_t v;
        uint8_t b[2];
    } u;
    u.v = v;
    return read_big_16(u.b);
}

static inline uint16_t htons(uint16_t v) {
    return ntohs(v);
}

static inline unsigned ntohl(uint32_t v) {
    union {
        uint32_t v;
        uint8_t b[4];
    } u;
    u.v = v;
    return (unsigned) read_big_32(u.b);
}

static inline uint32_t htonl(unsigned v) {
    return ntohl(v);
}
#endif



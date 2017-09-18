#pragma once
#include <stdint.h>

#define MS_TO_TICKS(ms) ((ms)*32768/1000)

void init_tick();
unsigned tick_count();
void set_sleep(unsigned target);

static inline unsigned tick_ms() {
	// use uint64_t for increased range during the conversion
	uint64_t tick = tick_count();
	tick *= 1000;
	tick /= 32768;
	return (unsigned) tick;
}



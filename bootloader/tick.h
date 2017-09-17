#pragma once
#include "am335x.h"
#include <stdint.h>

#define MS_TO_TICKS(ms) ((ms)*32768/1000)

static inline unsigned tick_count() {
	return HW_DMTIMER_1MS->TCRR;
}

static inline unsigned tick_ms() {
	// use uint64_t for increased range during the conversion
	uint64_t tick = HW_DMTIMER_1MS->TCRR;
	tick *= 1000;
	tick /= 32768;
	return (unsigned) tick;
}

static inline void set_sleep(unsigned target) {
	HW_DMTIMER_1MS->TMAR = target;
}

void setup_tick();

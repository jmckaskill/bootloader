#pragma once
#include "../printf/tinyprintf.h"

#define DEBUG_LOG_ENABLED

#ifdef DEBUG_LOG_ENABLED
void init_debug();
#define debugf tfp_printf
#else
static inline void init_debug() {}
#define debugf(fmt, ...) do{}while(0)
#endif

extern const char hex[];


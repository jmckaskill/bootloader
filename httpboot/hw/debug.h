#pragma once
#include "../tinyprintf/tinyprintf.h"

#ifndef NDEBUG
#define DEBUG_LOG_ENABLED
#endif

#ifdef DEBUG_LOG_ENABLED
void init_debug();
#define debugf tfp_printf
#else
static inline void init_debug() {}
#define debugf(...) do{}while(0)
#endif



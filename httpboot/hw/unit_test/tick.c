#include "../tick.h"
#include "unit_test.h"

unsigned test_tick;

void init_tick() {}
unsigned tick_count() {
    return test_tick;
}

void set_sleep(unsigned target) {}
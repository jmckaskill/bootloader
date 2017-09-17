#pragma once

#include "am335x.h"

void init_interconnects();

static inline void enable_module(volatile unsigned *ctrl) {
	*ctrl |= HW_CM_MODULEMODE_ENABLE;
	while ((*ctrl & HW_CM_MODULEMODE_MASK) != HW_CM_MODULEMODE_ENABLE) {
	}
}

static inline void sw_wakeup(volatile unsigned *status) {
	*status |= HW_CM_CLKTRCTRL_SW_WKUP;
	while ((*status & HW_CM_CLKTRCTRL_MASK) != HW_CM_CLKTRCTRL_SW_WKUP){
	}
}

static inline void wait_for_functional(volatile unsigned *ctrl) {
	while ((*ctrl & HW_CM_IDLEST_MASK) != HW_CM_IDLEST_FUNCTIONAL) {
	}
}
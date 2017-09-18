#include "hw.h"

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

static inline void hw_enable_interrupt(enum hw_interrupt idx) {
	HW_INTC->BANK[idx >> 5].MIR_CLEAR |= 1 << ((unsigned)idx & 0x1F);
}

static inline void hw_disable_interrupt(enum hw_interrupt idx) {
	HW_INTC->BANK[idx >> 5].MIR_SET |= 1 << ((unsigned)idx & 0x1F);
}

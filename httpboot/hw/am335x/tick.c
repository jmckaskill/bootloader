#include "../tick.h"
#include "../board.h"
#include "common.h"

void init_tick() {
	enable_module(&HW_CM_WAKEUP->TIMER1_CLKCTRL);
	wait_for_functional(&HW_CM_WAKEUP->TIMER1_CLKCTRL);
	while (!(HW_CM_WAKEUP->CLKSTCTRL & HW_CM_WAKEUP_TIMER1_GCLK)) {}

	HW_DMTIMER_1MS->TIOCP_CFG |= HW_1MS_CFG_SOFT_RESET;
	while (!(HW_DMTIMER_1MS->TISTAT & HW_1MS_TISTAT_RESET_DONE)) {}

	// Setup the clock to use the 32k external clock with no division
	HW_CM_DPLL->CLKSEL_TIMER1MS_CLK = HW_CLKSEL_TIMER1MS_32K_EXT;

	// enable the interrupt
	HW_DMTIMER_1MS->TIER |= HW_1MS_INT_COMPARE;
	hw_enable_interrupt(HW_INT_TINT1_1MS);

	set_sleep(tick_count() + MS_TO_TICKS(1000));

	// enable the timer in compare mode. this will trigger an interrupt whenever
	// the internal timer matches the comparison register
	HW_DMTIMER_1MS->TCLR |= HW_1MS_TCLR_COMPARE_EN | HW_1MS_TCLR_START;
}

unsigned tick_count() {
	return HW_DMTIMER_1MS->TCRR;
}

void set_sleep(unsigned target) {
	HW_DMTIMER_1MS->TMAR = target;
}

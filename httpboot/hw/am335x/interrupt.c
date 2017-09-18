#include "../debug.h"
#include "../tick.h"
#include "interrupt.h"
#include "hw.h"

void interrupt() {
	unsigned hwint = HW_INTC->SIR_IRQ & HW_INTC_ACTIVE_IRQ_MASK;
	debugf("%u interrupt %u\n", tick_ms(), hwint);

	switch (hwint) {
	case HW_INT_TINT1_1MS:
		on_tick();
		// reset the timer interrupt
		HW_DMTIMER_1MS->TISR |= HW_1MS_INT_COMPARE;
		set_sleep(tick_count() + MS_TO_TICKS(1000));
		break;
	case HW_INT_ETH_TX:
		process_eth_tx_interrupt();
		break;
	case HW_INT_ETH_RX:
		process_eth_rx_interrupt();
		break;
	case HW_INT_ETH_MISC:
		process_eth_misc_interrupt();
		break;
	}

	HW_INTC->CONTROL = HW_INTC_NEXT_IRQ;
}

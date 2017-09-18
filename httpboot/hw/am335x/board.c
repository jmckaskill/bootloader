#include "../board.h"
#include "common.h"

void init_board() {
	// enable L3 & L3S clocks in the peripheral power domain
	enable_module(&HW_CM_PER->L3);
	enable_module(&HW_CM_PER->L3_instr);
	sw_wakeup(&HW_CM_PER->L3_status);
	sw_wakeup(&HW_CM_PER->OCPWP_status);
	sw_wakeup(&HW_CM_PER->L3S_status);
	wait_for_functional(&HW_CM_PER->L3);
	wait_for_functional(&HW_CM_PER->L3_instr);
	while (!(HW_CM_PER->L3_status & HW_CM_L3_GCLK)) {}
	while (!(HW_CM_PER->OCPWP_status & HW_CM_OCPWP_L3_GCLK)) {}
	while (!(HW_CM_PER->L3S_status & HW_CM_L3S_GCLK)) {}

	// enable L3 clocks in the wakeup power domain
	enable_module(&HW_CM_WAKEUP->CONTROL_CLKCTRL);
	sw_wakeup(&HW_CM_WAKEUP->CLKSTCTRL);
	sw_wakeup(&HW_CM_WAKEUP->L3_AON_CLKSTCTRL);
	wait_for_functional(&HW_CM_WAKEUP->CONTROL_CLKCTRL);
	while (!(HW_CM_WAKEUP->L3_AON_CLKSTCTRL & HW_CM_L3_AON_GCLK)) {}

	// enable L4 clocks in the wakeup power domain
	// no need to enable the L4 wakeup module - it can't be disabled
	wait_for_functional(&HW_CM_WAKEUP->L4WKUP_CLKCTRL);
	while (!(HW_CM_WAKEUP->CLKSTCTRL & HW_CM_WAKEUP_L4_GCLK)) {}
	while (!(HW_CM_WAKEUP->L4_WKUP_AON_CLKSTCTRL & HW_CM_L4_AON_GCLK)) {}
	//while (!(HW_CM_PER->OCPWP_status & HW_CM_OCPWP_L4_GCLK)) {}

	// enable L4LS clocks in the peripheral power domain
	enable_module(&HW_CM_PER->L4LS);
	sw_wakeup(&HW_CM_PER->L4LS_status);
	wait_for_functional(&HW_CM_PER->L4LS);
	while (!(HW_CM_PER->L4LS_status & HW_CM_L4LS_GCLK)) {}

	// enable L4HS clocks in the peripheral power domain
	enable_module(&HW_CM_PER->L4HS);
	sw_wakeup(&HW_CM_PER->L4HS_status);
	wait_for_functional(&HW_CM_PER->L4HS);
	while (!(HW_CM_PER->L4HS_status & HW_CM_L4HS_GCLK)) {}
}

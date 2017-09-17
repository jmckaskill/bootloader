#include "debug.h"
#include "board.h"

const char hex[] = "0123456789abcdef";

#ifdef DEBUG_LOG_ENABLED
enum uart_tx_select {
	TX_UART4,
	TX_UART5,
};

static void set_tx_uart(enum uart_tx_select tx) {
	enable_module(&HW_CM_WAKEUP->GPIO0_CLKCTRL);
	wait_for_functional(&HW_CM_WAKEUP->GPIO0_CLKCTRL);

	HW_CONTROL->conf_gpmc_ad[11] = HW_CONTROL_MUXMODE_7 | HW_CONTROL_DISABLE_PULL;
	HW_GPIO_0->INPUT_EN &= ~(1 << 27);

	switch (tx) {
	case TX_UART4:
		HW_GPIO_0->CLEARDATAOUT |= (1 << 27);
		break;
	case TX_UART5:
		HW_GPIO_0->SETDATAOUT |= (1 << 27);
		break;
	}
}

static void init_uart4() {
	set_tx_uart(TX_UART4);

	HW_CONTROL->conf_uart[0].rtsn = HW_CONTROL_MUXMODE_1 | HW_CONTROL_PULL_UP;
	HW_CONTROL->conf_uart[0].ctsn = HW_CONTROL_MUXMODE_1 | HW_CONTROL_PULL_UP | HW_CONTROL_RX_ACTIVE;

	// enable the module
	enable_module(&HW_CM_PER->UART4);
	wait_for_functional(&HW_CM_PER->UART4);
	while (!(HW_CM_PER->L4LS_status & HW_CM_L4LS_UART_GFCLK)) {}

	// reset the module
	HW_UART_4->SYSC |= HW_UART_SYSC_SOFTRESET;
	while (!(HW_UART_4->SYSS & HW_UART_SYSS_RESETDONE)) {}

	// enable the output to 115200 8N1
	HW_UART_4->LCR = HW_UART_LCR_CFGB;
	HW_UART_4_CFGB->DLH = 0x00;
	HW_UART_4_CFGB->DLL = 0x1A;
	HW_UART_4->LCR = HW_UART_LCR_LENGTH_8 | HW_UART_LCR_STOP_1;
	HW_UART_4->MDR1 = HW_UART_MDR1_MODE_UART16X;

	// at this point we are in operational mode
}

static void debug_putc(void* udata, char ch) {
	while ((HW_UART_4_OP->LSR & HW_UART_LSR_TX_EMPTY) != HW_UART_LSR_TX_EMPTY) {}
	HW_UART_4_OP->RHR_THR = ch;
}

void init_debug() {
    init_uart4();
	init_printf(NULL, &debug_putc);
}

#endif
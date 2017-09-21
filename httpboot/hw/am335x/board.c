#include "../board.h"
#include "common.h"

void init_cpu() {
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

void init_memory() {
	// setup the DDR clock for 400 MHz
	// per the setup on p1230
	// 1&2. switch pll to bypass
	HW_CM_WAKEUP->CLKMODE_DPLL_DDR = HW_CM_MN_BYPASS_MODE;
	while (!(HW_CM_WAKEUP->IDLEST_DPLL_DDR & HW_CM_IN_MN_BYPASS)){}

	// 3. configure multiply and divide values in clksel
	// CLK_M_OSC is 25 MHz. This is the external OSC0 input.
	// OSC1 is the RTC (32768Hz) oscillator
	// we want to run at 400 MHz, so use 400/25 x 25 MHz M_OSC
	HW_CM_WAKEUP->CLKSEL_DPLL_DDR = HW_CM_DPLL_MULT(400) | HW_CM_DPLL_DIV(25);
	
	// 4. configure the M2 divider
	// this is the secondary divider that allows changes without relocking
	// set to 1 as we want the raw 400 MHz fed to the DDR
	HW_CM_WAKEUP->DIV_M2_DPLL_DDR = HW_CM_M2_DIV(1);

	// 5&6. switch to lock mode
	HW_CM_WAKEUP->CLKMODE_DPLL_DDR = HW_CM_LOCK_MODE;
	while (!(HW_CM_WAKEUP->IDLEST_DPLL_DDR & HW_CM_DPLL_LOCKED)) {}

	// enable the EMIF memory controller
	enable_module(&HW_CM_PER->EMIF);
	wait_for_functional(&HW_CM_PER->EMIF);
	while (!(HW_CM_PER->L3_status & HW_CM_L3_EMIF_GCLK)) {}

	// setup VTP (voltage, temperature, pressure) macro
	HW_CONTROL->vtp_ctrl = 0;
	HW_CONTROL->vtp_ctrl = HW_VTP_FILTER_4;
	HW_CONTROL->vtp_ctrl |= HW_VTP_ENABLE;
	// toggle clrz to clear the counts
	HW_CONTROL->vtp_ctrl &= ~HW_VTP_CLRZ;
	HW_CONTROL->vtp_ctrl |= HW_VTP_CLRZ;
	while (!(HW_CONTROL->vtp_ctrl & HW_VTP_READY)) {}

	// setup the DDR phy delays for each line
	// the values here are taken from the TI CCS GEL file
	HW_DDR->CMD[0].SLAVE_RATIO = 0x80;
	HW_DDR->CMD[1].SLAVE_RATIO = 0x80;
	HW_DDR->CMD[2].SLAVE_RATIO = 0x80;
	HW_DDR->CMD[0].INVERT_CLKOUT = 0;
	HW_DDR->CMD[1].INVERT_CLKOUT = 0;
	HW_DDR->CMD[2].INVERT_CLKOUT = 0;
	HW_DDR->DATA[0].RD_DQS_SLAVE_RATIO = 0x38;
	HW_DDR->DATA[1].RD_DQS_SLAVE_RATIO = 0x38;
	HW_DDR->DATA[0].WR_DQS_SLAVE_RATIO = 0x44;
	HW_DDR->DATA[1].WR_DQS_SLAVE_RATIO = 0x44;
	HW_DDR->DATA[0].FIFO_WE_SLAVE_RATIO = 0x94;
	HW_DDR->DATA[1].FIFO_WE_SLAVE_RATIO = 0x94;
	HW_DDR->DATA[0].WR_DATA_SLAVE_RATIO = 0x7D;
	HW_DDR->DATA[1].WR_DATA_SLAVE_RATIO = 0x7D;

	// setup the I/O impedence and slew rate
	unsigned ddr_ioctl = HW_DDR_IOCTL_IMPEDENCE_3 
		| HW_DDR_IOCTL_SLEW_1
		| HW_DDR_IOCTL_CLK_IMPEDENCE_4
		| HW_DDR_IOCTL_CLK_SLEW_1;
	HW_CONTROL->ddr_cmd_ioctrl[0] = ddr_ioctl;
	HW_CONTROL->ddr_cmd_ioctrl[1] = ddr_ioctl;
	HW_CONTROL->ddr_cmd_ioctrl[2] = ddr_ioctl;
	HW_CONTROL->ddr_data_ioctrl[0] = ddr_ioctl;
	HW_CONTROL->ddr_data_ioctrl[1] = ddr_ioctl;

	// disable mDDR
	HW_CONTROL->ddr_io_ctrl &= ~HW_DDR_MDDR_SEL;

	// CKE controlled by EMIF
	HW_CONTROL->ddr_cke_ctrl = HW_DDR_CKE_EMIF_CONTROLLED;

	// setup read latency registers
	HW_EMIF->DDR_PHY_CTRL_1 = HW_DDR_PHY_DYNAMIC_POWER_DOWN
		| HW_DDR_PHY_READ_LATENCY(6);

	// Setup timing registers
	// target frequency is 400 MHz = 2.5 ns
	unsigned tim1 = HW_TIM_1_RP(6) // 13.75ns
		| HW_TIM_1_RCD(6) // 13.75ns
		| HW_TIM_1_WR(6) // 15ns
		| HW_TIM_1_RAS(14) // 35ns
		| HW_TIM_1_RC(20) // 48.75ns
		| HW_TIM_1_RRD(4) // 4 CK
		| HW_TIM_1_WTR(4); // 4 CK

	unsigned tim2 = HW_TIM_2_XP(3) // 3 CK
		| HW_TIM_2_ODT(3) // = CWL (5CK) + AL (0CK) - 2 = 3 CK
		| HW_TIM_2_XSNR(108) // = RFC (260ns) + 10ns = 270ns
		| HW_TIM_2_XSRD(512) // = tDLLK = 512 CK
		| HW_TIM_2_RTP(4) // 4 CK
		| HW_TIM_2_CKE(3); // 3 CK

	unsigned tim3 = HW_TIM_3_FIXED
		| HW_TIM_3_ZQCS(64) // 64 CK
		| HW_TIM_3_RFC(104); // 260 ns

	HW_EMIF->SDRAM_TIM_1 = tim1;
	HW_EMIF->SDRAM_TIM_1_SHDW = tim1;
	HW_EMIF->SDRAM_TIM_2 = tim2;
	HW_EMIF->SDRAM_TIM_2_SHDW = tim2;
	HW_EMIF->SDRAM_TIM_3 = tim3;
	HW_EMIF->SDRAM_TIM_3_SHDW = tim3;

	// 2.5ns * 3120 = 7.8us
	HW_EMIF->SDRAM_REF_CTRL = HW_REFRESH_RATE(3120);
	HW_EMIF->SDRAM_REF_CTRL_SHDW = HW_REFRESH_RATE(3120);

	HW_EMIF->ZQ_CONFIG = HW_ZQ_CS0_ONLY
		| HW_ZQ_ZQCL_ON_EXIT
		| HW_ZQ_ZQINIT_MUL(2) // 606ms * 2 = 1.21s
		| HW_ZQ_ZQCL_MUL(4) // 152ms * 4 = 606ms
		| HW_ZQ_ZQCS_MUL(0x4BE4); // refresh rate 7.8us * 0x4BE4 = 152ms

	HW_EMIF->SDRAM_CONFIG = HW_SDCFG_DDR3
		| HW_SDCFG_IBANK_LOWER_BITS
		| HW_SDCFG_TERM_RZQ_4
		| HW_SDCFG_DIFFERENTIAL_DQS
		| HW_SDCFG_DYNAMIC_ODT_RZQ_2
		| HW_SDCFG_DRIVE_RZQ_6
		| HW_SDCFG_CWL_5
		| HW_SDCFG_16_BIT
		| HW_SDCFG_CL_6
		| HW_SDCFG_ROW_15_BITS
		| HW_SDCFG_IBANK_8_BANKS
		| HW_SDCFG_EBANK_1_CHIP
		| HW_SDCFG_PAGE_10_BITS;

	// at this point the EMIF engine is enabled
	// wait for it to be ready
	while (!(HW_EMIF->STATUS & HW_EMIF_READY)) {}
}


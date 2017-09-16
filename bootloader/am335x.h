#include <stdint.h>
#include <assert.h>

// status regs
#define HW_CM_CLKTRCTRL_MASK 3
#define HW_CM_CLKTRCTRL_NO_SLEEP 0
#define HW_CM_CLKTRCTRL_SW_SLEEP 1
#define HW_CM_CLKTRCTRL_SW_WKUP 2

// L4LS CLKACTIVITY fields
#define HW_CM_L4LS_TIMER6_GCLK (1 << 28)
#define HW_CM_L4LS_TIMER5_GCLK (1 << 27)
#define HW_CM_L4LS_SPI_GCLK (1 << 25)
#define HW_CM_L4LS_I2C_FCLK (1 << 24)
#define HW_CM_L4LS_GPIO_3_GDBCLK (1 << 21)
#define HW_CM_L4LS_GPIO_2_GDBCLK (1 << 20)
#define HW_CM_L4LS_GPIO_1_GDBCLK (1 << 19)
#define HW_CM_L4LS_LCDC_GCLK (1 << 17)
#define HW_CM_L4LS_TIMER4_GCLK (1 << 16)
#define HW_CM_L4LS_TIMER3_GCLK (1 << 15)
#define HW_CM_L4LS_TIMER2_GCLK (1 << 14)
#define HW_CM_L4LS_TIMER7_GCLK (1 << 13)
#define HW_CM_L4LS_CAN_CLK (1 << 11)
#define HW_CM_L4LS_UART_GFCLK (1 << 10)
#define HW_CM_L4LS_GCLK (1 << 8)

// L3S CLKACTIVITY fields
#define HW_CM_L3S_GCLK (1 << 3)

// L3 CLKACTIVITY fields
#define HW_CM_L3_MCASP_GCLK (1 << 7)
#define HW_CM_L3_CPTS_REF_GCLK (1 << 6)
#define HW_CM_L3_GCLK (1 << 4)
#define HW_CM_L3_MMC_FCLK (1 << 3)
#define HW_CM_L3_EMIF_GCLK (1 << 2)

// L4HS clock activity fields
#define HW_CM_L4HS_CPSW_5MHZ_GCLK (1 << 6)
#define HW_CM_L4HS_CPSW_50MHZ_GCLK (1 << 6)
#define HW_CM_L4HS_CPSW_25MHZ_GCLK (1 << 6)
#define HW_CM_L4HS_GCLK (1 << 3)

// OCPWP clock activity fields
#define HW_CM_OCPWP_L4_GCLK (1 << 5)
#define HW_CM_OCPWP_L3_GCLK (1 << 4)

// PRU ICSS clock activity fields
#define HW_CM_PRU_ICSS_UART_GCLK (1 << 6)
#define HW_CM_PRU_ICSS_IEP_GCLK (1 << 5)
#define HW_CM_PRU_ICSS_OCP_GCLK (1 << 4)

// CPSW clock activity fields
#define HW_CM_CPSW_125MHZ_GCLK (1 << 4)

// LCDC clock activity fields
#define HW_CM_LCDC_L4_GCLK (1 << 5)
#define HW_CM_LCDC_L3_GCLK (1 << 4)

// 24MHz clock activity fields
#define HW_CM_CLK_24MHZ_GCLK (1 << 4)

// control regs
#define HW_CM_MODULEMODE_MASK 3
#define HW_CM_MODULEMODE_ENABLE 2
#define HW_CM_MODULEMODE_DISABLE 0

#define HW_CM_IDLEST_MASK (3 << 16)
#define HW_CM_IDLEST_FUNCTIONAL (0 << 16)
#define HW_CM_IDLEST_TRANSITION (1 << 16)
#define HW_CM_IDLEST_IDLE (2 << 16)
#define HW_CM_IDLEST_DISABLED (3 << 16)

struct HW_CM_PER {
	volatile unsigned L4LS_status;
	volatile unsigned L3S_status;
	char pad1[4];
	volatile unsigned L3_status;
	char pad2[4];
	volatile unsigned CPGMAC0;
	volatile unsigned LCDC;
	volatile unsigned USB0;
	char pad3[4];
	volatile unsigned TPTC0;
	volatile unsigned EMIF;
	volatile unsigned OCMCRAM;
	volatile unsigned GPMC;
	volatile unsigned MCASP0;
	volatile unsigned UART5;
	volatile unsigned MMC0;
	volatile unsigned ELM;
	volatile unsigned I2C2;
	volatile unsigned I2C1;
	volatile unsigned SPI0;
	volatile unsigned SPI1;
	char pad4[12];
	volatile unsigned L4LS;
	char pad5[4];
	volatile unsigned MCASP1;
	volatile unsigned UART1;
	volatile unsigned UART2;
	volatile unsigned UART3;
	volatile unsigned UART4;
	volatile unsigned TIMER7;
	volatile unsigned TIMER2;
	volatile unsigned TIMER3;
	volatile unsigned TIMER4;
	char pad6[0x20];
	volatile unsigned GPIO1;
	volatile unsigned GPIO2;
	volatile unsigned GPIO3;
	char pad7[4];
	volatile unsigned TPCC;
	volatile unsigned DCAN0;
	volatile unsigned DCAN1;
	char pad8[4];
	volatile unsigned EPWMSS1;
	char pad9[4];
	volatile unsigned EPWMSS0;
	volatile unsigned EPWMSS2;
	volatile unsigned L3_instr;
	volatile unsigned L3;
	volatile unsigned IEEE5000;
	volatile unsigned PRU_ICSS;
	volatile unsigned TIMER5;
	volatile unsigned TIMER6;
	volatile unsigned MMC1;
	volatile unsigned MMC2;
	volatile unsigned TPTC1;
	volatile unsigned TPTC2;
	char pad10[8];
	volatile unsigned SPINLOCK;
	volatile unsigned MAILBOX0;
	char pad11[8];
	volatile unsigned L4HS_status;
	volatile unsigned L4HS;
	char pad12[8];
	volatile unsigned OCPWP_status;
	volatile unsigned OCPWP;
	char pad[12];
	volatile unsigned PRU_ICSS_status;
	volatile unsigned CPSW_status;
	volatile unsigned LCDC_status;
	volatile unsigned CLKDIV32K;
	volatile unsigned CLK_24MHZ;
};

static_assert(sizeof(struct HW_CM_PER) == 0x154, "padding");
extern struct HW_CM_PER HW_CM_PER;

// wakeup clocks
#define HW_CM_WAKEUP_ADC_FCLK (1 << 14)
#define HW_CM_WAKEUP_TIMER1_GCLK (1 << 13)
#define HW_CM_WAKEUP_UART0_GFCLK (1 << 12)
#define HW_CM_WAKEUP_I2C0_GFCLK (1 << 11)
#define HW_CM_WAKEUP_TIMER0_GCLK (1 << 10)
#define HW_CM_WAKEUP_GPIO0_GDBCLK (1 << 8)
#define HW_CM_WAKEUP_WDT1_GCLK (1 << 4)
#define HW_CM_WAKEUP_SR_SYSCLK (1 << 3)
#define HW_CM_WAKEUP_L4_GCLK (1 << 2)

// L3 always on clocks
#define HW_CM_L3_AON_DEBUG_CLKA (1 << 4)
#define HW_CM_L3_AON_GCLK (1 << 3)
#define HW_CM_L3_AON_DBGSYSCLK (1 << 2)

// L4 always on clocks
#define HW_CM_L4_AON_GCLK (1 << 2)

struct HW_CM_WAKEUP {
	volatile unsigned CLKSTCTRL; //   (0x0)
	volatile unsigned CONTROL_CLKCTRL; //   (0x4)
	volatile unsigned GPIO0_CLKCTRL; //   (0x8)
	volatile unsigned L4WKUP_CLKCTRL; //   (0xc)
	volatile unsigned TIMER0_CLKCTRL; //   (0x10)
	volatile unsigned DEBUGSS_CLKCTRL; //   (0x14)
	volatile unsigned L3_AON_CLKSTCTRL; //   (0x18)
	volatile unsigned AUTOIDLE_DPLL_MPU; //   (0x1c)
	volatile unsigned IDLEST_DPLL_MPU; //   (0x20)
	volatile unsigned SSC_DELTAMSTEP_DPLL_MPU; //   (0x24)
	volatile unsigned SSC_MODFREQDIV_DPLL_MPU; //   (0x28)
	volatile unsigned CLKSEL_DPLL_MPU; //   (0x2c)
	volatile unsigned AUTOIDLE_DPLL_DDR; //   (0x30)
	volatile unsigned IDLEST_DPLL_DDR; //   (0x34)
	volatile unsigned SSC_DELTAMSTEP_DPLL_DDR; //   (0x38)
	volatile unsigned SSC_MODFREQDIV_DPLL_DDR; //   (0x3c)
	volatile unsigned CLKSEL_DPLL_DDR; //   (0x40)
	volatile unsigned AUTOIDLE_DPLL_DISP; //   (0x44)
	volatile unsigned IDLEST_DPLL_DISP; //   (0x48)
	volatile unsigned SSC_DELTAMSTEP_DPLL_DISP; //   (0x4c)
	volatile unsigned SSC_MODFREQDIV_DPLL_DISP; //   (0x50)
	volatile unsigned CLKSEL_DPLL_DISP; //   (0x54)
	volatile unsigned AUTOIDLE_DPLL_CORE; //   (0x58)
	volatile unsigned IDLEST_DPLL_CORE; //   (0x5c)
	volatile unsigned SSC_DELTAMSTEP_DPLL_CORE; //   (0x60)
	volatile unsigned SSC_MODFREQDIV_DPLL_CORE; //   (0x64)
	volatile unsigned CLKSEL_DPLL_CORE; //   (0x68)
	volatile unsigned AUTOIDLE_DPLL_PER; //   (0x6c)
	volatile unsigned IDLEST_DPLL_PER; //   (0x70)
	volatile unsigned SSC_DELTAMSTEP_DPLL_PER; //   (0x74)
	volatile unsigned SSC_MODFREQDIV_DPLL_PER; //   (0x78)
	volatile unsigned CLKDCOLDO_DPLL_PER; //   (0x7c)
	volatile unsigned DIV_M4_DPLL_CORE; //   (0x80)
	volatile unsigned DIV_M5_DPLL_CORE; //   (0x84)
	volatile unsigned CLKMODE_DPLL_MPU; //   (0x88)
	volatile unsigned CLKMODE_DPLL_PER; //   (0x8c)
	volatile unsigned CLKMODE_DPLL_CORE; //   (0x90)
	volatile unsigned CLKMODE_DPLL_DDR; //   (0x94)
	volatile unsigned CLKMODE_DPLL_DISP; //   (0x98)
	volatile unsigned CLKSEL_DPLL_PERIPH; //   (0x9c)
	volatile unsigned DIV_M2_DPLL_DDR; //   (0xa0)
	volatile unsigned DIV_M2_DPLL_DISP; //   (0xa4)
	volatile unsigned DIV_M2_DPLL_MPU; //   (0xa8)
	volatile unsigned DIV_M2_DPLL_PER; //   (0xac)
	volatile unsigned WKUP_M3_CLKCTRL; //   (0xb0)
	volatile unsigned UART0_CLKCTRL; //   (0xb4)
	volatile unsigned I2C0_CLKCTRL; //   (0xb8)
	volatile unsigned ADC_TSC_CLKCTRL; //   (0xbc)
	volatile unsigned SMARTREFLEX0_CLKCTRL; //   (0xc0)
	volatile unsigned TIMER1_CLKCTRL; //   (0xc4)
	volatile unsigned SMARTREFLEX1_CLKCTRL; //   (0xc8)
	volatile unsigned L4_WKUP_AON_CLKSTCTRL; //   (0xcc)
	volatile unsigned WDT0_CLKCTRL; //   (0xd0)
	volatile unsigned WDT1_CLKCTRL; //   (0xd4)
	volatile unsigned DIV_M6_DPLL_CORE; //   (0xd8)
};

static_assert(sizeof(struct HW_CM_WAKEUP) == 0xDC, "padding");
extern struct HW_CM_WAKEUP HW_CM_WAKEUP;

#define HW_CLKSEL_TIMER1MS_32K_EXT (4 << 0)

struct HW_CM_DPLL {
    char pad0[4];
    volatile unsigned CLKSEL_TIMER7_CLK; // 0x4
    volatile unsigned CLKSEL_TIMER2_CLK; // 0x8
    volatile unsigned CLKSEL_TIMER3_CLK; // 0xc
    volatile unsigned CLKSEL_TIMER4_CLK; // 0x10
    volatile unsigned CM_MAC_CLKSEL; // 0x14
    volatile unsigned CLKSEL_TIMER5_CLK; // 0x18
    volatile unsigned CLKSEL_TIMER6_CLK; // 0x1c
    volatile unsigned CM_CPTS_RFT_CLKSEL; // 0x20
    char pad1[4];
    volatile unsigned CLKSEL_TIMER1MS_CLK; // 0x28
    volatile unsigned CLKSEL_GFX_FCLK; // 0x2c
    volatile unsigned CLKSEL_ICSS_OCP_CLK; // 0x30
    volatile unsigned CLKSEL_LCDC_PIXEL_CLK; // 0x34
    volatile unsigned CLKSEL_WDT1_CLK; // 0x38
    volatile unsigned CLKSEL_GPIO0_DBCLK; // 0x3c
};

static_assert(sizeof(struct HW_CM_DPLL) == 0x40, "padding");
extern struct HW_CM_DPLL HW_CM_DPLL;

#define HW_CONTROL_MUXMODE_MASK 7
#define HW_CONTROL_MUXMODE_0 0
#define HW_CONTROL_MUXMODE_1 1
#define HW_CONTROL_MUXMODE_2 2
#define HW_CONTROL_MUXMODE_3 3
#define HW_CONTROL_MUXMODE_4 4
#define HW_CONTROL_MUXMODE_5 5
#define HW_CONTROL_MUXMODE_6 6
#define HW_CONTROL_MUXMODE_7 7

#define HW_CONTROL_DISABLE_PULL (1 << 3)
#define HW_CONTROL_PULL_DOWN (1 << 4)
#define HW_CONTROL_PULL_UP (0 << 4)
#define HW_CONTROL_RX_ACTIVE (1 << 5)
#define HW_CONTROL_SLEW_FAST (0 << 6)
#define HW_CONTROL_SLEW_SLOW (1 << 6)

#define HW_CONTROL_GMII_1 (0 << 0)
#define HW_CONTROL_GMII_2 (0 << 2)

struct HW_CONTROL {
    volatile unsigned revision; // 0x0)
    volatile unsigned hwinfo; // 0x4)
    char pad0[8];
    volatile unsigned sysconfig; // 0x10)
    char pad1[0x2C];
    volatile unsigned status; // 0x40)
    volatile unsigned bootstat; // 0x44)
    char pad2[0xB8];
    volatile unsigned sec_ctrl; // 0x100)
    volatile unsigned sec_sw; // 0x104)
    volatile unsigned sec_emu; // 0x108)
    char pad3[4];
    volatile unsigned secure_emif_sdram_config; // 0x110)
    char pad4[0x314];
    volatile unsigned core_sldo_ctrl; // 0x428)
    volatile unsigned mpu_sldo_ctrl; // 0x42c)
    char pad5[0x14];
    volatile unsigned clk32kdivratio_ctrl; // 0x444)
    volatile unsigned bandgap_ctrl; // 0x448)
    volatile unsigned bandgap_trim; // 0x44c)
    char pad6[8];
    volatile unsigned pll_clkinpulow_ctrl; // 0x458)
    char pad7[12];
    volatile unsigned mosc_ctrl; // 0x468)
    char pad8[4];
    volatile unsigned deepsleep_ctrl; // 0x470)
    char pad9[0x98];
    volatile unsigned dpll_pwr_sw_state; // 0x50C
    char pad10[0xF0];
    volatile unsigned device_id; // 0x600)
    volatile unsigned dev_feature; // 0x604)
    volatile unsigned init_priority[2]; // 0x608,0x60C
    char pad11[4];
    volatile unsigned tptc_cfg; // 0x614)
    char pad12[8];
    struct {
        volatile unsigned ctrl;
        volatile unsigned sts;
    } usb[2]; // 0x620 - 0x630
    struct {
        volatile unsigned lo;
        volatile unsigned hi;
    } mac[2]; // 0x630 - 0x640
    char pad13[4];
    volatile unsigned dcan_raminit; // 0x644)
    volatile unsigned usb_wkup_ctrl; // 0x648)
    char pad14[4];
    volatile unsigned gmii_sel; // 0x650)
    char pad15[0x10];
    volatile unsigned pwmss_ctrl; // 0x664)
    char pad16[8];
    volatile unsigned mreqprio[2]; // 0x670-0x678
    char pad17[0x18];
    volatile unsigned hw_event_sel_grp[4]; // 0x690-0x6A0
    volatile unsigned smrt_ctrl; // 0x6a0)
    volatile unsigned sabtooth_hw_debug_sel; // 0x6a4)
    volatile unsigned sabtooth_hw_dbg_info; // 0x6a8)
    char pad18[0xC4];
    volatile unsigned vdd_mpu_opp_050; // 0x770
    volatile unsigned vdd_mpu_opp_100; // 0x774
    volatile unsigned vdd_mpu_opp_120; // 0x778
    volatile unsigned vdd_mpu_opp_turbo; // 0x77c)
    char pad19[0x38];
    volatile unsigned vdd_core_opp_050; // 0x7B8
    volatile unsigned vdd_core_opp_100; // 0x7BC
    char pad20[0x10];
    volatile unsigned bb_scale; // 0x7d0)
    char pad21[0x20];
    volatile unsigned usb_vid_pid; // 0x7f4)
    char pad50[4];
    volatile unsigned efuse_sma; // 0x7fc)
    volatile unsigned conf_gpmc_ad[16]; // 0x800-0x840
    volatile unsigned conf_gpmc_a[12]; // 0x840-0x870
    volatile unsigned conf_gpmc_wait0; // 0x870)
    volatile unsigned conf_gpmc_wpn; // 0x874)
    volatile unsigned conf_gpmc_be1n; // 0x878)
    volatile unsigned conf_gpmc_csn[4]; // 0x87c-0x88C
    volatile unsigned conf_gpmc_clk; // 0x88c)
    volatile unsigned conf_gpmc_advn_ale; // 0x890)
    volatile unsigned conf_gpmc_oen_ren; // 0x894)
    volatile unsigned conf_gpmc_wen; // 0x898)
    volatile unsigned conf_gpmc_be0n_cle; // 0x89c)
    volatile unsigned conf_lcd_data[16]; //0x8a0-0x8E0
    volatile unsigned conf_lcd_vsync; // 0x8e0)
    volatile unsigned conf_lcd_hsync; // 0x8e4)
    volatile unsigned conf_lcd_pclk; // 0x8e8)
    volatile unsigned conf_lcd_ac_bias_en; // 0x8ec)
    volatile unsigned conf_mmc0_dat3; // 0x8f0)
    volatile unsigned conf_mmc0_dat2; // 0x8f4)
    volatile unsigned conf_mmc0_dat1; // 0x8f8)
    volatile unsigned conf_mmc0_dat0; // 0x8fc)
    volatile unsigned conf_mmc0_clk; // 0x900)
    volatile unsigned conf_mmc0_cmd; // 0x904)
    volatile unsigned conf_mii1_col; // 0x908)
    volatile unsigned conf_mii1_crs; // 0x90c)
    volatile unsigned conf_mii1_rxerr; // 0x910)
    volatile unsigned conf_mii1_txen; // 0x914)
    volatile unsigned conf_mii1_rxdv; // 0x918)
    volatile unsigned conf_mii1_txd3; // 0x91c)
    volatile unsigned conf_mii1_txd2; // 0x920)
    volatile unsigned conf_mii1_txd1; // 0x924)
    volatile unsigned conf_mii1_txd0; // 0x928)
    volatile unsigned conf_mii1_txclk; // 0x92c)
    volatile unsigned conf_mii1_rxclk; // 0x930)
    volatile unsigned conf_mii1_rxd3; // 0x934)
    volatile unsigned conf_mii1_rxd2; // 0x938)
    volatile unsigned conf_mii1_rxd1; // 0x93c)
    volatile unsigned conf_mii1_rxd0; // 0x940)
    volatile unsigned conf_rmii1_refclk; // 0x944)
    volatile unsigned conf_mdio_data; // 0x948)
    volatile unsigned conf_mdio_clk; // 0x94c)
    volatile unsigned conf_spi0_sclk; // 0x950)
    volatile unsigned conf_spi0_d0; // 0x954)
    volatile unsigned conf_spi0_d1; // 0x958)
    volatile unsigned conf_spi0_cs0; // 0x95c)
    volatile unsigned conf_spi0_cs1; // 0x960)
    volatile unsigned conf_ecap0_in_pwm0_out; // 0x964)
    struct {
        volatile unsigned ctsn;
        volatile unsigned rtsn;
        volatile unsigned rxd;
        volatile unsigned txd;
    } conf_uart[2]; // 0x968-0x988
    volatile unsigned conf_i2c0_sda; // 0x988)
    volatile unsigned conf_i2c0_scl; // 0x98c)
    volatile unsigned conf_mcasp0_aclkx; // 0x990)
    volatile unsigned conf_mcasp0_fsx; // 0x994)
    volatile unsigned conf_mcasp0_axr0; // 0x998)
    volatile unsigned conf_mcasp0_ahclkr; // 0x99c)
    volatile unsigned conf_mcasp0_aclkr; // 0x9a0)
    volatile unsigned conf_mcasp0_fsr; // 0x9a4)
    volatile unsigned conf_mcasp0_axr1; // 0x9a8)
    volatile unsigned conf_mcasp0_ahclkx; // 0x9ac)
    volatile unsigned conf_xdma_event_intr[2]; // 0x9b0-0x9B8
    volatile unsigned conf_nresetin_out; // 0x9b8)
    volatile unsigned conf_porz; // 0x9bc)
    volatile unsigned conf_nnmi; // 0x9c0)
    char pad23[12];
    volatile unsigned conf_tms; // 0x9d0)
    volatile unsigned conf_tdi; // 0x9d4)
    volatile unsigned conf_tdo; // 0x9d8)
    volatile unsigned conf_tck; // 0x9dc)
    volatile unsigned conf_ntrst; // 0x9e0)
    volatile unsigned conf_emu[2]; // 0x9e4-0x9EC
    char pad24[12];
    volatile unsigned conf_rtc_porz; // 0x9f8)
    volatile unsigned conf_pmic_power_en; // 0x9fc)
    volatile unsigned conf_ext_wakeup; // 0xa00)
    char pad25[0x18];
    volatile unsigned conf_usb0_drvvbus; // 0xa1c
    char pad26[0x14];
    volatile unsigned conf_usb1_drvvbus; // 0xa34
    char pad27[0x3C8];
    volatile unsigned cqdetect_status; // 0xe00)
    volatile unsigned ddr_io_ctrl; // 0xe04)
    char pad28[4];
    volatile unsigned vtp_ctrl; // 0xe0c)
    char pad29[4];
    volatile unsigned vref_ctrl; // 0xe14)
    char pad30[0x178];
    volatile unsigned tpcc_evt_mux_0_3; // 0xf90)
    volatile unsigned tpcc_evt_mux_4_7; // 0xf94)
    volatile unsigned tpcc_evt_mux_8_11; // 0xf98)
    volatile unsigned tpcc_evt_mux_12_15; // 0xf9c)
    volatile unsigned tpcc_evt_mux_16_19; // 0xfa0)
    volatile unsigned tpcc_evt_mux_20_23; // 0xfa4)
    volatile unsigned tpcc_evt_mux_24_27; // 0xfa8)
    volatile unsigned tpcc_evt_mux_28_31; // 0xfac)
    volatile unsigned tpcc_evt_mux_32_35; // 0xfb0)
    volatile unsigned tpcc_evt_mux_36_39; // 0xfb4)
    volatile unsigned tpcc_evt_mux_40_43; // 0xfb8)
    volatile unsigned tpcc_evt_mux_44_47; // 0xfbc)
    volatile unsigned tpcc_evt_mux_48_51; // 0xfc0)
    volatile unsigned tpcc_evt_mux_52_55; // 0xfc4)
    volatile unsigned tpcc_evt_mux_56_59; // 0xfc8)
    volatile unsigned tpcc_evt_mux_60_63; // 0xfcc)
    volatile unsigned timer_evt_capt; // 0xfd0)
    volatile unsigned ecap_evt_capt; // 0xfd4)
    volatile unsigned adc_evt_capt; // 0xfd8)
    char pad31[0x24];
    volatile unsigned reset_iso; // 0x1000)
    char pad32[0x314];
    volatile unsigned dpll_pwr_sw_ctrl; // 0x1318
    volatile unsigned ddr_cke_ctrl; // 0x131c)
    volatile unsigned sma2; // 0x1320
    volatile unsigned m3_txev_eoi; // 0x1324)
    volatile unsigned ipc_msg_reg[8]; // 0x1328-0x1348
    char pad33[0xBC];
    volatile unsigned ddr_cmd_ioctrl[3]; // 0x1404-0x1410
    char pad34[0x30];
    volatile unsigned ddr_data_ioctrl[2]; // 0x1440-0x1448
};

static_assert(sizeof(struct HW_CONTROL) == 0x1448, "padding");
extern struct HW_CONTROL HW_CONTROL;

#define HW_UART_SYSC_SOFTRESET (1 << 1)
#define HW_UART_SYSS_RESETDONE (1 << 0)

#define HW_UART_MDR1_MODE_UART16X (0 << 0)
#define HW_UART_MDR1_MODE_SIR (1 << 0)
#define HW_UART_MDR1_MODE_UART16X_AUTO_BAUD (2 << 0)
#define HW_UART_MDR1_MODE_UART13X (3 << 0)
#define HW_UART_MDR1_MODE_MIR (4 << 0)
#define HW_UART_MDR1_MODE_FIR (5 << 0)
#define HW_UART_MDR1_MODE_CIR (6 << 0)
#define HW_UART_MDR1_MODE_DISABLE (7 << 0)

#define HW_UART_LCR_CFGA 0x80
#define HW_UART_LCR_CFGB 0xBF

#define HW_UART_LCR_LENGTH_5 (0 << 0)
#define HW_UART_LCR_LENGTH_6 (1 << 0)
#define HW_UART_LCR_LENGTH_7 (2 << 0)
#define HW_UART_LCR_LENGTH_8 (3 << 0)

#define HW_UART_LCR_STOP_1 (0 << 2)
#define HW_UART_LCR_STOP_1_5 (1 << 2)

#define HW_UART_LCR_PARITY_EN (1 << 3)

#define HW_UART_LCR_PARITY_ODD (0 << 4)
#define HW_UART_LCR_PARITY_EVEN (1 << 4)

#define HW_UART_LSR_TX_EMPTY (3 << 5)

#define HW_UART_MCR_LOOPBACK_EN (1 << 4)

struct HW_UART {
    char pad0[0x0C];
    volatile unsigned LCR; // 0x0C line control register
    char pad1[0x10];
    volatile unsigned MDR1; // 0x20 mode definition register 1
    volatile unsigned MDR2; // 0x24 mode definition register 2
    char pad2[0x18];
    volatile unsigned SCR; // 0x40 supplementary control register
    volatile unsigned SSR; // 0x44 supplementary status register
    char pad3[0x8];
    volatile unsigned MVR; // 0x50 module version register
    volatile unsigned SYSC; // 0x54 system configuration register
    volatile unsigned SYSS; // 0x58 system status register
    volatile unsigned WER; // 0x5C wakeup enable register
    char pad4[0x4];
    volatile unsigned RXFIFO_LVL; // 0x64
    volatile unsigned TXFIFO_LVL; // 0x68
    volatile unsigned IER2; // 0x6C interrupt enable register 2
    volatile unsigned ISR2; // 0x70 interrupt status register 2
    volatile unsigned FREQ_SEL; // 0x74
    char pad5[0x8];
    volatile unsigned MDR3; // 0x80 mode definition register 3
    volatile unsigned TX_DMA_THRESHOLD; // 0x84
};

static_assert(sizeof(struct HW_UART) == 0x88, "padding");
extern struct HW_UART HW_UART_4;

struct HW_UART_CFGA {
    volatile unsigned DLL; // 0x00 divisor latches low
    volatile unsigned DLH; // 0x04 divisor latches high
    volatile unsigned IIR_FCR; // 0x08 interrupt identification register & FIFO control register
    char pad1[0x4];
    volatile unsigned MCR; // 0x10 modem control register
    volatile unsigned LSR; // 0x14 line status register
    volatile unsigned MSR_TCR; // 0x18 modem status register & transmission control register
    volatile unsigned SPR_TLR; // 0x1C scratchpad register & trigger level register
    char pad2[0x18];
    volatile unsigned UASR; // 0x38 UART autobauding status register
};

static_assert(sizeof(struct HW_UART_CFGA) == 0x3C, "padding");
extern struct HW_UART_CFGA HW_UART_4_CFGA;

struct HW_UART_CFGB {
    volatile unsigned DLL; // 0x00 divisor latches low
    volatile unsigned DLH; // 0x04 divisor latches high
    volatile unsigned EFR; // 0x08 enhanced feature register
    char pad1[0x4];
    volatile unsigned XON1_ADDR; // 0x10 XON1
    volatile unsigned XON2_ADDR; // 0x14 XON2
    volatile unsigned XOFF1_TCR; // 0x18 XOFF1 & transmission control register
    volatile unsigned XOFF2_TLR; // 0x1C XOFF2 & trigger level register
    char pad2[0x18];
    volatile unsigned UASR; // 0x38 UART autobauding status register
};

static_assert(sizeof(struct HW_UART_CFGB) == 0x3C, "padding");
extern struct HW_UART_CFGB HW_UART_4_CFGB;

struct HW_UART_OP {
    volatile unsigned RHR_THR; // 0x00 receive/transmit holding register
    volatile unsigned IER; // 0x04 interrupt enable register
    volatile unsigned IIR_FCR; // 0x08 interrupt identification register & FIFO control register
    char pad1[0x4];
    volatile unsigned MCR; // 0x10 modem control register
    volatile unsigned LSR; // 0x14 line status register
    volatile unsigned MSR_TCR; // 0x18 modem status register & transmission control register
    volatile unsigned SPR_TLR; // 0x1C scratchpad register & trigger level register
};

static_assert(sizeof(struct HW_UART_OP) == 0x20, "padding");
extern struct HW_UART_OP HW_UART_4_OP;

#define HW_ALE_CONTROL_EN (1 << 31)
#define HW_ALE_CONTROL_CLEAR (1 << 30)
#define HW_ALE_CONTROL_BYPASS (1 << 4)

#define HW_ALE_PORT_FORWARD (3 << 0)

struct HW_CPSW_ALE {
    volatile unsigned IDVER; // 0x0
    char pad1[4];
    volatile unsigned CONTROL; // 0x8
    char pad2[4];
    volatile unsigned PRESCALE; // 0x10
    char pad3[4];
    volatile unsigned UNKNOWN_VLAN; // 0x18
    char pad4[4];
    volatile unsigned TBLCTL; // 0x20
    char pad5[0x10];
    volatile unsigned TBLW_2; // 0x34
    volatile unsigned TBLW_1; // 0x38
    volatile unsigned TBLW_0; // 0x3C
    volatile unsigned PORTCTL[6]; // 0x40-0x58
};

static_assert(sizeof(struct HW_CPSW_ALE) == 0x58, "padding");
extern struct HW_CPSW_ALE HW_CPSW_ALE;

#define HW_CPDMA_TX_EN (1 << 0)
#define HW_CPDMA_RX_EN (1 << 0)
#define HW_CPDMA_SOFT_RESET (1 << 0)

#define HW_CPDMA_TX_PEND_0 (1 << 0)
#define HW_CPDMA_RX_PEND_0 (1 << 0)
#define HW_CPDMA_RX_THRESH_PEND_0 (1 << 8)

#define HW_CPDMA_HOST_PEND (1 << 1)
#define HW_CPDMA_STAT_PEND (1 << 0)

#define HW_CPDMA_EOI_RX 1
#define HW_CPDMA_EOI_TX 2

struct HW_CPSW_CPDMA {
    volatile unsigned TX_IDVER; // 0x0
    volatile unsigned TX_CONTROL; // 0x4
    volatile unsigned TX_TEARDOWN; // 0x8
    char pad1[4];
    volatile unsigned RX_IDVER; // 0x10
    volatile unsigned RX_CONTROL; // 0x14
    volatile unsigned RX_TEARDOWN; // 0x18
    volatile unsigned CPDMA_SOFT_RESET; // 0x1c
    volatile unsigned DMACONTROL; // 0x20
    volatile unsigned DMASTATUS; // 0x24
    volatile unsigned RX_BUFFER_OFFSET; // 0x28
    volatile unsigned EMCONTROL; // 0x2c
    volatile unsigned TX_PRI_RATE[8]; // 0x30-0x50
    char pad2[0x30];
    volatile unsigned TX_INTSTAT_RAW; // 0x80
    volatile unsigned TX_INTSTAT_MASKED; // 0x84
    volatile unsigned TX_INTMASK_SET; // 0x88
    volatile unsigned TX_INTMASK_CLEAR; // 0x8c
    volatile unsigned CPDMA_IN_VECTOR; // 0x90
    volatile unsigned CPDMA_EOI_VECTOR; // 0x94
    char pad3[8];
    volatile unsigned RX_INTSTAT_RAW; // 0xa0
    volatile unsigned RX_INTSTAT_MASKED; // 0xa4
    volatile unsigned RX_INTMASK_SET; // 0xa8
    volatile unsigned RX_INTMASK_CLEAR; // 0xac
    volatile unsigned DMA_INTSTAT_RAW; // 0xb0
    volatile unsigned DMA_INTSTAT_MASKED; // 0xb4
    volatile unsigned DMA_INTMASK_SET; // 0xb8
    volatile unsigned DMA_INTMASK_CLEAR; // 0xbc
    volatile unsigned RX_PENDTHRESH[8]; // 0xc0-0xE0
    volatile unsigned RX_FREEBUFFER[8]; // 0xe0-0x100
};

static_assert(sizeof(struct HW_CPSW_CPDMA) == 0x100, "padding");
static struct HW_CPSW_CPDMA *HW_CPSW_CPDMA = (struct HW_CPSW_CPDMA*) 0x4A100800;

// flags for both rx & tx
#define HW_ETH_START_OF_PKT (1 << 31)
#define HW_ETH_END_OF_PKT (1 << 30)
#define HW_ETH_FULL_PKT (3 << 30)
#define HW_ETH_OWNED_BY_PORT (1 << 29)
#define HW_ETH_END_OF_QUEUE (1 << 28)
#define HW_ETH_TEARDOWN_COMPLETE (1 << 27)
#define HW_ETH_PASS_CRC (1 << 26)

#define HW_ETH_PORT_MASK (3 << 16)
#define HW_ETH_PORT_1 (1 << 16)
#define HW_ETH_PORT_2 (2 << 16)

// flags for tx only
#define HW_ETH_TO_PORT_EN (1 << 20)

// flags for rx only
#define HW_ETH_PKT_TOO_LONG (1 << 25)
#define HW_ETH_PKT_TOO_SHORT (1 << 24)
#define HW_ETH_CONTROL_FRAME (1 << 23)
#define HW_ETH_OVERRUN (1 << 22)

#define HW_ETH_ERROR_MASK (3 << 20)
#define HW_ETH_NO_ERROR (0 << 20)
#define HW_ETH_CRC_ERROR (1 << 20)
#define HW_ETH_CODE_ERROR (2 << 20)
#define HW_ETH_ALIGN_ERROR (3 << 20)

#define HW_ETH_HAS_VLAN (1 << 19)

#define HW_ETH_MAX_LEN (1500+18)

struct hw_buffer_descriptor {
    struct hw_buffer_descriptor *next;
    void *buffer;
    volatile unsigned offset_buf_len;
    volatile unsigned flags_pkt_len;
};

extern struct hw_buffer_descriptor HW_BUFFER_DESCRIPTORS[512];
static_assert(sizeof(struct hw_buffer_descriptor) == 16, "padding");
static_assert(sizeof(HW_BUFFER_DESCRIPTORS) == 0x2000, "padding");

struct HW_CPSW_STATERAM {
    volatile struct hw_buffer_descriptor * volatile TX_HDP[8]; // 0-0x20
    volatile struct hw_buffer_descriptor * volatile RX_HDP[8]; // 0x20-0x40
    volatile struct hw_buffer_descriptor * volatile TX_CP[8]; // 0x40-0x60
    volatile struct hw_buffer_descriptor * volatile RX_CP[8]; // 0x60-0x80
};

static_assert(sizeof(struct HW_CPSW_STATERAM) == 0x80, "padding");
extern struct HW_CPSW_STATERAM HW_CPSW_STATERAM;

struct HW_CPSW_CPTS {
    volatile unsigned IDVER; // 0x00
    volatile unsigned CONTROL; // 0x04
    volatile unsigned RFTCLK_SEL; // 0x08
    volatile unsigned TS_PUSH; // 0x0c
    volatile unsigned TS_LOAD_VAL; // 0x10
    volatile unsigned TS_LOAD_EN; // 0x14
    char pad1[8];
    volatile unsigned INTSTAT_RAW; // 0x20
    volatile unsigned INTSTAT_MASKED; // 0x24
    volatile unsigned INT_ENABLE; // 0x28
    char pad2[4];
    volatile unsigned EVENT_POP; // 0x30
    volatile unsigned EVENT_LOW; // 0x34
    volatile unsigned EVENT_HIGH; // 0x38
};

static_assert(sizeof(struct HW_CPSW_CPTS) == 0x3C, "padding");
extern struct HW_CPSW_CPTS HW_CPSW_CPTS;

struct HW_CPSW_PORT_0 {
    volatile unsigned CONTROL; // 0x0
    char pad1[4];
    volatile unsigned MAX_BLKS; // 0x8
    volatile unsigned BLK_CNT; // 0xc
    volatile unsigned TX_IN_CTL; // 0x10
    volatile unsigned PORT_VLAN; // 0x14
    volatile unsigned TX_PRI_MAP; // 0x18
    volatile unsigned CPDMA_TX_PRI_MAP; // 0x1c
    volatile unsigned CPDMA_RX_CH_MAP; // 0x20
    char pad2[12];
    volatile unsigned RX_DSCP_PRI_MAP[8]; // 0x30-0x50
};

struct HW_CPSW_PORT {
    volatile unsigned CONTROL; // 0x0
    char pad1[4];
    volatile unsigned MAX_BLKS; // 0x8
    volatile unsigned BLK_CNT; // 0xc
    volatile unsigned TX_IN_CTL; // 0x10
    volatile unsigned PORT_VLAN; // 0x14
    volatile unsigned TX_PRI_MAP; // 0x18
    volatile unsigned TS_SEQ_MTYPE; // 0x1C
    volatile unsigned SA_LO; // 0x20
    volatile unsigned SA_HI; // 0x24
    volatile unsigned SEND_PERCENT; // 0x28
    char pad2[4];
    volatile unsigned RX_DSCP_PRI_MAP[8]; // 0x30-0x50
};

static_assert(sizeof(struct HW_CPSW_PORT_0) == 0x50, "padding");
static_assert(sizeof(struct HW_CPSW_PORT) == 0x50, "padding");
extern struct HW_CPSW_PORT_0 HW_CPSW_PORT_0;
extern struct HW_CPSW_PORT HW_CPSW_PORT_1;
extern struct HW_CPSW_PORT HW_CPSW_PORT_2;

#define HW_SL_MACCONTROL_GIGABIT (1 << 7)
#define HW_SL_MACCONTROL_TX_PACE (1 << 6)
#define HW_SL_MACCONTROL_MII_EN (1 << 5)
#define HW_SL_MACCONTROL_TX_FLOW_CONTROL (1 << 4)
#define HW_SL_MACCONTROL_RX_FLOW_CONTROL (1 << 3)
#define HW_SL_MACCONTROL_FULL_DUPLEX (1 << 0)

#define HW_SL_MACSTATUS_IDLE (1 << 31)
#define HW_SL_MACSTATUS_GIGABIT (1 << 4)
#define HW_SL_MACSTATUS_FULL_DUPLEX (1 << 3)
#define HW_SL_MACSTATUS_RX_FLOW_CONTROL (1 << 1)
#define HW_SL_MACSTATUS_TX_FLOW_CONTROL (1 << 0)

#define HW_SL_SOFT_RESET (1 << 0)

struct HW_CPSW_SL {
    volatile unsigned IDVER; // 0x0
    volatile unsigned MACCONTROL; // 0x4
    volatile unsigned MACSTATUS; // 0x8
    volatile unsigned SOFT_RESET; // 0xc
    volatile unsigned RX_MAXLEN; // 0x10
    volatile unsigned BOFFTEST; // 0x14
    volatile unsigned RX_PAUSE; // 0x18
    volatile unsigned TX_PAUSE; // 0x1c
    volatile unsigned EMCONTROL; // 0x20
    volatile unsigned RX_PRI_MAP; // 0x24
    volatile unsigned TX_GAP; // 0x28
};

static_assert(sizeof(struct HW_CPSW_SL) == 0x2C, "padding");
extern struct HW_CPSW_SL HW_CPSW_SL_1;
extern struct HW_CPSW_SL HW_CPSW_SL_2;

#define HW_SS_SOFT_RESET (1 << 0)
#define HW_SS_STAT_EN_PORT_0 (1 << 0)
#define HW_SS_STAT_EN_PORT_1 (1 << 1)
#define HW_SS_STAT_EN_PORT_2 (1 << 2)

struct HW_CPSW_SS {
    volatile unsigned ID_VER; // 0x0
    volatile unsigned CONTROL; // 0x4
    volatile unsigned SOFT_RESET; // 0x8
    volatile unsigned STAT_PORT_EN; // 0xc
    volatile unsigned PTYPE; // 0x10
    volatile unsigned SOFT_IDLE; // 0x14
    volatile unsigned THRU_RATE; // 0x18
    volatile unsigned GAP_THRESH; // 0x1c
    volatile unsigned TX_START_WDS; // 0x20
    volatile unsigned FLOW_CONTROL; // 0x24
    volatile unsigned VLAN_LTYPE; // 0x28
    volatile unsigned TS_LTYPE; // 0x2c
    volatile unsigned DLR_LTYPE; // 0x30
};

static_assert(sizeof(struct HW_CPSW_SS) == 0x34, "padding");
extern struct HW_CPSW_SS HW_CPSW_SS;

// these are only valid for phys using RGMII in-band status
// MACCONTROL.EXT_EN needs to be enabled

#define HW_WR_RGMII_2_FULL_DUPLEX (1 << 7)

#define HW_WR_RGMII_2_SPEED_MASK (3 << 5)
#define HW_WR_RGMII_2_SPEED_10 (0 << 5)
#define HW_WR_RGMII_2_SPEED_100 (1 << 5)
#define HW_WR_RGMII_2_SPEED_1000 (2 << 5)

#define HW_WR_RGMII_2_LINK (1 << 0)

#define HW_WR_RGMII_1_FULL_DUPLEX (1 << 3)

#define HW_WR_RGMII_1_SPEED_MASK (3 << 1)
#define HW_WR_RGMII_1_SPEED_10 (0 << 1)
#define HW_WR_RGMII_1_SPEED_100 (1 << 1)
#define HW_WR_RGMII_1_SPEED_1000 (2 << 1)

#define HW_WR_RGMII_1_LINK (1 << 0)

#define HW_WR_CONTROL_NO_STANDBY (1 << 2)
#define HW_WR_CONTROL_NO_IDLE (1 << 0)

#define HW_WR_MISC_INT_EVENT (1 << 4)
#define HW_WR_MISC_INT_STAT (1 << 3)
#define HW_WR_MISC_INT_HOST (1 << 2)
#define HW_WR_MISC_INT_MDIO_LINK (1 << 1)
#define HW_WR_MISC_INT_MDIO_USER (1 << 0)

#define HW_WR_MISC_INT_DONE 3

#define HW_WR_INT_PACE_RX_0 (1 << 16)

struct HW_CPSW_WR {
    volatile unsigned IDVER; // 0x0
    volatile unsigned SOFT_RESET; // 0x04
    volatile unsigned CONTROL; // 0x08
    volatile unsigned INT_CONTROL; // 0x0c
    struct {
        volatile unsigned RX_THRESH;
        volatile unsigned RX;
        volatile unsigned TX;
        volatile unsigned MISC;
    } PORT_INT_ENABLE[3]; // 0x10-0x40
    struct {
        volatile unsigned RX_THRESH;
        volatile unsigned RX;
        volatile unsigned TX;
        volatile unsigned MISC;
    } PORT_INT_STATUS[3]; // 0x40-0x70
    struct {
        volatile unsigned RX;
        volatile unsigned TX;
    } PORT_INT_MAX[3]; // 0x70-0x88
    volatile unsigned RGMII_CTL; // 0x88
};

static_assert(sizeof(struct HW_CPSW_WR) == 0x8C, "padding");
extern struct HW_CPSW_WR HW_CPSW_WR;


#define HW_MDIO_CONTROL_IDLE (1 << 31)
#define HW_MDIO_CONTROL_EN (1 << 30)
#define HW_MDIO_CLKDIV(x) (((x) & 0xFFFF) << 0)
#define HW_MDIO_CLKDIV_MASK 0xFFFF

// useraccess fields
#define HW_MDIO_GO (1 << 31)
#define HW_MDIO_ACK (1 << 29)
#define HW_MDIO_WRITE(phy, key, val) ((3 << 30) | (((key) & 0x1F) << 21) | (((phy) & 0x1F) << 16) | ((val) & 0xFFFF))
#define HW_MDIO_READ(phy, key) ((2 << 30) | (((key) & 0x1F) << 21) | (((phy) & 0x1F) << 16))
#define HW_MDIO_VALUE(reg) ((reg) & 0xFFFF)

// userphysel fields
#define HW_MDIO_USE_LINK_PIN (1 << 7)
#define HW_MDIO_LINK_INT_EN (1 << 6)
#define HW_MDIO_USERPHYSEL_ADDR(x) ((x) & 0xF)

struct HW_MDIO {
    volatile unsigned REVID; // 0x0
    volatile unsigned CONTROL; // 0x4
    volatile unsigned ALIVE; // 0x8
    volatile unsigned LINK; // 0xC
    volatile unsigned LINKINTRAW; // 0x10
    volatile unsigned LINKINTMASKED; // 0x14
    char pad1[8];
    volatile unsigned USERINTRAW; // 0x20
    volatile unsigned USERINTMASKED; // 0x24
    volatile unsigned USERINTMASKSET; // 0x28
    volatile unsigned USERINTMASKCLEAR; // 0x2C
    char pad2[0x50];
    volatile unsigned USERACCESS0; // 0x80
    volatile unsigned USERPHYSEL0; // 0x84
    volatile unsigned USERACCESS1; // 0x88
    volatile unsigned USERPHYSEL1; // 0x8C
};

#define MDIO_BASIC_CONTROL 0
#define MDIO_BASIC_STATUS 1

#define MDIO_CTL_SOFT_RESET (1 << 15)
#define MDIO_CTL_AUTO_NEGOTIATION (1 << 12)
#define MDIO_CTL_RESTART_AUTO (1 << 9)

#define MDIO_STS_AUTO_COMPLETE (1 << 5)
#define MDIO_STS_LINK_UP (1 << 2)

static_assert(sizeof(struct HW_MDIO) == 0x90, "padding");
extern struct HW_MDIO HW_MDIO;

#define HW_INTC_NEXT_IRQ (1 << 0)
#define HW_INTC_ACTIVE_IRQ_MASK 0xFF

enum hw_interrupt {
    HW_INT_ETH_RX_THRESHOLD = 40,
    HW_INT_ETH_RX = 41,
    HW_INT_ETH_TX = 42,
    HW_INT_ETH_MISC = 43,
    HW_INT_TINT1_1MS = 67,
};

struct HW_INTC {
    volatile unsigned REVISION; // 0x0
    char pad[12];
    volatile unsigned SYSCONFIG; // 0x10
    volatile unsigned SYSSTATUS; // 0x14
    char pad2[0x28];
    volatile unsigned SIR_IRQ; // 0x40
    volatile unsigned SIR_FIQ; // 0x44
    volatile unsigned CONTROL; // 0x48
    volatile unsigned PROTECTION; // 0x4c
    volatile unsigned IDLE; // 0x50
    char pad3[12];
    volatile unsigned IRQ_PRIORITY; // 0x60
    volatile unsigned FIQ_PRIORITY; // 0x64
    volatile unsigned THRESHOLD; // 0x68
    char pad4[0x14];
    struct {
        volatile unsigned ITR; // 0x00
        volatile unsigned MIR; // 0x04
        volatile unsigned MIR_CLEAR; // 0x08
        volatile unsigned MIR_SET; // 0x0C
        volatile unsigned ISR_SET; // 0x10
        volatile unsigned ISR_CLEAR; // 0x14
        volatile unsigned PENDING_IRQ; // 0x18
        volatile unsigned PENDING_FIQ; // 0x1C
    } BANK[4]; // 0x80-0x100

    volatile unsigned ILR[128]; // 0x100-0x300
};

static_assert(sizeof(struct HW_INTC) == 0x300, "padding");
extern struct HW_INTC HW_INTC;

struct HW_GPIO {
    volatile unsigned REVISION; // 0x0
    char pad1[12];
    volatile unsigned SYSCONFIG; // 0x10
    char pad2[12];
	volatile unsigned EOI; // 0x20
    volatile unsigned IRQSTATUS_RAW[2]; // 0x24-0x2C
    volatile unsigned IRQSTATUS[2]; // 0x2C-0x34
    volatile unsigned IRQSTATUS_SET[2]; // 0x34-0x3C
    volatile unsigned IRQSTATUS_CLR[2]; // 0x3C-0x44
    volatile unsigned IRQWAKEN[2]; // 0x44-0x4C
    char pad3[0xC8];
    volatile unsigned SYSSTATUS; // 0x114
    char pad4[0x18];
    volatile unsigned CTRL; // 0x130
    volatile unsigned INPUT_EN; // 0x134
    volatile unsigned DATAIN; // 0x138
    volatile unsigned DATAOUT; // 0x13C
    volatile unsigned LEVELDETECT[2]; // 0x140-0x148
    volatile unsigned RISINGDETECT; // 0x148
    volatile unsigned FALLINGDETECT; // 0x14C
    volatile unsigned DEBOUNCENABLE; // 0x150
    volatile unsigned DEBOUNCINGTIME; // 0x154
    char pad5[0x38];
    volatile unsigned CLEARDATAOUT; // 0x190
    volatile unsigned SETDATAOUT; // 0x194
};

static_assert(sizeof(struct HW_GPIO) == 0x198, "padding");
extern struct HW_GPIO HW_GPIO_0;
extern struct HW_GPIO HW_GPIO_1;
extern struct HW_GPIO HW_GPIO_2;
extern struct HW_GPIO HW_GPIO_3;

#define HW_1MS_CFG_SOFT_RESET (1 << 1)
#define HW_1MS_TISTAT_RESET_DONE (1 << 0)

#define HW_1MS_INT_COMPARE (1 << 0)
#define HW_1MS_TCLR_COMPARE_EN (1 << 6)
#define HW_1MS_TCLR_START (1 << 0)

struct HW_DMTIMER_1MS {
    volatile unsigned TIDR; // 0x0
    char pad1[12];
    volatile unsigned TIOCP_CFG; // 0x10
    volatile unsigned TISTAT; // 0x14
    volatile unsigned TISR; // 0x18
    volatile unsigned TIER; // 0x1c
    volatile unsigned TWER; // 0x20
    volatile unsigned TCLR; // 0x24
    volatile unsigned TCRR; // 0x28
    volatile unsigned TLDR; // 0x2c
    volatile unsigned TTGR; // 0x30
    volatile unsigned TWPS; // 0x34
    volatile unsigned TMAR; // 0x38
    volatile unsigned TCAR1; // 0x3C
    volatile unsigned TSICR; // 0x40
    volatile unsigned TCAR2; // 0x44
    volatile unsigned TPIR; // 0x48
    volatile unsigned TNIR; // 0x4c
    volatile unsigned TCVR; // 0x50
    volatile unsigned TOCR; // 0x54
    volatile unsigned TOWR; // 0x58
};

static_assert(sizeof(struct HW_DMTIMER_1MS) == 0x5C, "padding");
extern struct HW_DMTIMER_1MS HW_DMTIMER_1MS;


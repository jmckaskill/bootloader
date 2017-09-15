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
	volatile uint32_t L4LS_status;
	volatile uint32_t L3S_status;
	char pad1[4];
	volatile uint32_t L3_status;
	char pad2[4];
	volatile uint32_t CPGMAC0;
	volatile uint32_t LCDC;
	volatile uint32_t USB0;
	char pad3[4];
	volatile uint32_t TPTC0;
	volatile uint32_t EMIF;
	volatile uint32_t OCMCRAM;
	volatile uint32_t GPMC;
	volatile uint32_t MCASP0;
	volatile uint32_t UART5;
	volatile uint32_t MMC0;
	volatile uint32_t ELM;
	volatile uint32_t I2C2;
	volatile uint32_t I2C1;
	volatile uint32_t SPI0;
	volatile uint32_t SPI1;
	char pad4[12];
	volatile uint32_t L4LS;
	char pad5[4];
	volatile uint32_t MCASP1;
	volatile uint32_t UART1;
	volatile uint32_t UART2;
	volatile uint32_t UART3;
	volatile uint32_t UART4;
	volatile uint32_t TIMER7;
	volatile uint32_t TIMER2;
	volatile uint32_t TIMER3;
	volatile uint32_t TIMER4;
	char pad6[0x20];
	volatile uint32_t GPIO1;
	volatile uint32_t GPIO2;
	volatile uint32_t GPIO3;
	char pad7[4];
	volatile uint32_t TPCC;
	volatile uint32_t DCAN0;
	volatile uint32_t DCAN1;
	char pad8[4];
	volatile uint32_t EPWMSS1;
	char pad9[4];
	volatile uint32_t EPWMSS0;
	volatile uint32_t EPWMSS2;
	volatile uint32_t L3_instr;
	volatile uint32_t L3;
	volatile uint32_t IEEE5000;
	volatile uint32_t PRU_ICSS;
	volatile uint32_t TIMER5;
	volatile uint32_t TIMER6;
	volatile uint32_t MMC1;
	volatile uint32_t MMC2;
	volatile uint32_t TPTC1;
	volatile uint32_t TPTC2;
	char pad10[8];
	volatile uint32_t SPINLOCK;
	volatile uint32_t MAILBOX0;
	char pad11[8];
	volatile uint32_t L4HS_status;
	volatile uint32_t L4HS;
	char pad12[8];
	volatile uint32_t OCPWP_status;
	volatile uint32_t OCPWP;
	char pad[12];
	volatile uint32_t PRU_ICSS_status;
	volatile uint32_t CPSW_status;
	volatile uint32_t LCDC_status;
	volatile uint32_t CLKDIV32K;
	volatile uint32_t CLK_24MHZ;
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
	volatile uint32_t CLKSTCTRL; //   (0x0)
	volatile uint32_t CONTROL_CLKCTRL; //   (0x4)
	volatile uint32_t GPIO0_CLKCTRL; //   (0x8)
	volatile uint32_t L4WKUP_CLKCTRL; //   (0xc)
	volatile uint32_t TIMER0_CLKCTRL; //   (0x10)
	volatile uint32_t DEBUGSS_CLKCTRL; //   (0x14)
	volatile uint32_t L3_AON_CLKSTCTRL; //   (0x18)
	volatile uint32_t AUTOIDLE_DPLL_MPU; //   (0x1c)
	volatile uint32_t IDLEST_DPLL_MPU; //   (0x20)
	volatile uint32_t SSC_DELTAMSTEP_DPLL_MPU; //   (0x24)
	volatile uint32_t SSC_MODFREQDIV_DPLL_MPU; //   (0x28)
	volatile uint32_t CLKSEL_DPLL_MPU; //   (0x2c)
	volatile uint32_t AUTOIDLE_DPLL_DDR; //   (0x30)
	volatile uint32_t IDLEST_DPLL_DDR; //   (0x34)
	volatile uint32_t SSC_DELTAMSTEP_DPLL_DDR; //   (0x38)
	volatile uint32_t SSC_MODFREQDIV_DPLL_DDR; //   (0x3c)
	volatile uint32_t CLKSEL_DPLL_DDR; //   (0x40)
	volatile uint32_t AUTOIDLE_DPLL_DISP; //   (0x44)
	volatile uint32_t IDLEST_DPLL_DISP; //   (0x48)
	volatile uint32_t SSC_DELTAMSTEP_DPLL_DISP; //   (0x4c)
	volatile uint32_t SSC_MODFREQDIV_DPLL_DISP; //   (0x50)
	volatile uint32_t CLKSEL_DPLL_DISP; //   (0x54)
	volatile uint32_t AUTOIDLE_DPLL_CORE; //   (0x58)
	volatile uint32_t IDLEST_DPLL_CORE; //   (0x5c)
	volatile uint32_t SSC_DELTAMSTEP_DPLL_CORE; //   (0x60)
	volatile uint32_t SSC_MODFREQDIV_DPLL_CORE; //   (0x64)
	volatile uint32_t CLKSEL_DPLL_CORE; //   (0x68)
	volatile uint32_t AUTOIDLE_DPLL_PER; //   (0x6c)
	volatile uint32_t IDLEST_DPLL_PER; //   (0x70)
	volatile uint32_t SSC_DELTAMSTEP_DPLL_PER; //   (0x74)
	volatile uint32_t SSC_MODFREQDIV_DPLL_PER; //   (0x78)
	volatile uint32_t CLKDCOLDO_DPLL_PER; //   (0x7c)
	volatile uint32_t DIV_M4_DPLL_CORE; //   (0x80)
	volatile uint32_t DIV_M5_DPLL_CORE; //   (0x84)
	volatile uint32_t CLKMODE_DPLL_MPU; //   (0x88)
	volatile uint32_t CLKMODE_DPLL_PER; //   (0x8c)
	volatile uint32_t CLKMODE_DPLL_CORE; //   (0x90)
	volatile uint32_t CLKMODE_DPLL_DDR; //   (0x94)
	volatile uint32_t CLKMODE_DPLL_DISP; //   (0x98)
	volatile uint32_t CLKSEL_DPLL_PERIPH; //   (0x9c)
	volatile uint32_t DIV_M2_DPLL_DDR; //   (0xa0)
	volatile uint32_t DIV_M2_DPLL_DISP; //   (0xa4)
	volatile uint32_t DIV_M2_DPLL_MPU; //   (0xa8)
	volatile uint32_t DIV_M2_DPLL_PER; //   (0xac)
	volatile uint32_t WKUP_M3_CLKCTRL; //   (0xb0)
	volatile uint32_t UART0_CLKCTRL; //   (0xb4)
	volatile uint32_t I2C0_CLKCTRL; //   (0xb8)
	volatile uint32_t ADC_TSC_CLKCTRL; //   (0xbc)
	volatile uint32_t SMARTREFLEX0_CLKCTRL; //   (0xc0)
	volatile uint32_t TIMER1_CLKCTRL; //   (0xc4)
	volatile uint32_t SMARTREFLEX1_CLKCTRL; //   (0xc8)
	volatile uint32_t L4_WKUP_AON_CLKSTCTRL; //   (0xcc)
	volatile uint32_t WDT0_CLKCTRL; //   (0xd0)
	volatile uint32_t WDT1_CLKCTRL; //   (0xd4)
	volatile uint32_t DIV_M6_DPLL_CORE; //   (0xd8)
};

static_assert(sizeof(struct HW_CM_WAKEUP) == 0xDC, "padding");
extern struct HW_CM_WAKEUP HW_CM_WAKEUP;

#define HW_CLKSEL_TIMER1MS_32K_EXT (4 << 0)

struct HW_CM_DPLL {
    char pad0[4];
    volatile uint32_t CLKSEL_TIMER7_CLK; // 0x4
    volatile uint32_t CLKSEL_TIMER2_CLK; // 0x8
    volatile uint32_t CLKSEL_TIMER3_CLK; // 0xc
    volatile uint32_t CLKSEL_TIMER4_CLK; // 0x10
    volatile uint32_t CM_MAC_CLKSEL; // 0x14
    volatile uint32_t CLKSEL_TIMER5_CLK; // 0x18
    volatile uint32_t CLKSEL_TIMER6_CLK; // 0x1c
    volatile uint32_t CM_CPTS_RFT_CLKSEL; // 0x20
    char pad1[4];
    volatile uint32_t CLKSEL_TIMER1MS_CLK; // 0x28
    volatile uint32_t CLKSEL_GFX_FCLK; // 0x2c
    volatile uint32_t CLKSEL_ICSS_OCP_CLK; // 0x30
    volatile uint32_t CLKSEL_LCDC_PIXEL_CLK; // 0x34
    volatile uint32_t CLKSEL_WDT1_CLK; // 0x38
    volatile uint32_t CLKSEL_GPIO0_DBCLK; // 0x3c
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
    volatile uint32_t revision; // 0x0)
    volatile uint32_t hwinfo; // 0x4)
    char pad0[8];
    volatile uint32_t sysconfig; // 0x10)
    char pad1[0x2C];
    volatile uint32_t status; // 0x40)
    volatile uint32_t bootstat; // 0x44)
    char pad2[0xB8];
    volatile uint32_t sec_ctrl; // 0x100)
    volatile uint32_t sec_sw; // 0x104)
    volatile uint32_t sec_emu; // 0x108)
    char pad3[4];
    volatile uint32_t secure_emif_sdram_config; // 0x110)
    char pad4[0x314];
    volatile uint32_t core_sldo_ctrl; // 0x428)
    volatile uint32_t mpu_sldo_ctrl; // 0x42c)
    char pad5[0x14];
    volatile uint32_t clk32kdivratio_ctrl; // 0x444)
    volatile uint32_t bandgap_ctrl; // 0x448)
    volatile uint32_t bandgap_trim; // 0x44c)
    char pad6[8];
    volatile uint32_t pll_clkinpulow_ctrl; // 0x458)
    char pad7[12];
    volatile uint32_t mosc_ctrl; // 0x468)
    char pad8[4];
    volatile uint32_t deepsleep_ctrl; // 0x470)
    char pad9[0x98];
    volatile uint32_t dpll_pwr_sw_state; // 0x50C
    char pad10[0xF0];
    volatile uint32_t device_id; // 0x600)
    volatile uint32_t dev_feature; // 0x604)
    volatile uint32_t init_priority[2]; // 0x608,0x60C
    char pad11[4];
    volatile uint32_t tptc_cfg; // 0x614)
    char pad12[8];
    struct {
        volatile uint32_t ctrl;
        volatile uint32_t sts;
    } usb[2]; // 0x620 - 0x630
    struct {
        volatile uint32_t lo;
        volatile uint32_t hi;
    } mac[2]; // 0x630 - 0x640
    char pad13[4];
    volatile uint32_t dcan_raminit; // 0x644)
    volatile uint32_t usb_wkup_ctrl; // 0x648)
    char pad14[4];
    volatile uint32_t gmii_sel; // 0x650)
    char pad15[0x10];
    volatile uint32_t pwmss_ctrl; // 0x664)
    char pad16[8];
    volatile uint32_t mreqprio[2]; // 0x670-0x678
    char pad17[0x18];
    volatile uint32_t hw_event_sel_grp[4]; // 0x690-0x6A0
    volatile uint32_t smrt_ctrl; // 0x6a0)
    volatile uint32_t sabtooth_hw_debug_sel; // 0x6a4)
    volatile uint32_t sabtooth_hw_dbg_info; // 0x6a8)
    char pad18[0xC4];
    volatile uint32_t vdd_mpu_opp_050; // 0x770
    volatile uint32_t vdd_mpu_opp_100; // 0x774
    volatile uint32_t vdd_mpu_opp_120; // 0x778
    volatile uint32_t vdd_mpu_opp_turbo; // 0x77c)
    char pad19[0x38];
    volatile uint32_t vdd_core_opp_050; // 0x7B8
    volatile uint32_t vdd_core_opp_100; // 0x7BC
    char pad20[0x10];
    volatile uint32_t bb_scale; // 0x7d0)
    char pad21[0x20];
    volatile uint32_t usb_vid_pid; // 0x7f4)
    char pad50[4];
    volatile uint32_t efuse_sma; // 0x7fc)
    volatile uint32_t conf_gpmc_ad[16]; // 0x800-0x840
    volatile uint32_t conf_gpmc_a[12]; // 0x840-0x870
    volatile uint32_t conf_gpmc_wait0; // 0x870)
    volatile uint32_t conf_gpmc_wpn; // 0x874)
    volatile uint32_t conf_gpmc_be1n; // 0x878)
    volatile uint32_t conf_gpmc_csn[4]; // 0x87c-0x88C
    volatile uint32_t conf_gpmc_clk; // 0x88c)
    volatile uint32_t conf_gpmc_advn_ale; // 0x890)
    volatile uint32_t conf_gpmc_oen_ren; // 0x894)
    volatile uint32_t conf_gpmc_wen; // 0x898)
    volatile uint32_t conf_gpmc_be0n_cle; // 0x89c)
    volatile uint32_t conf_lcd_data[16]; //0x8a0-0x8E0
    volatile uint32_t conf_lcd_vsync; // 0x8e0)
    volatile uint32_t conf_lcd_hsync; // 0x8e4)
    volatile uint32_t conf_lcd_pclk; // 0x8e8)
    volatile uint32_t conf_lcd_ac_bias_en; // 0x8ec)
    volatile uint32_t conf_mmc0_dat3; // 0x8f0)
    volatile uint32_t conf_mmc0_dat2; // 0x8f4)
    volatile uint32_t conf_mmc0_dat1; // 0x8f8)
    volatile uint32_t conf_mmc0_dat0; // 0x8fc)
    volatile uint32_t conf_mmc0_clk; // 0x900)
    volatile uint32_t conf_mmc0_cmd; // 0x904)
    volatile uint32_t conf_mii1_col; // 0x908)
    volatile uint32_t conf_mii1_crs; // 0x90c)
    volatile uint32_t conf_mii1_rxerr; // 0x910)
    volatile uint32_t conf_mii1_txen; // 0x914)
    volatile uint32_t conf_mii1_rxdv; // 0x918)
    volatile uint32_t conf_mii1_txd3; // 0x91c)
    volatile uint32_t conf_mii1_txd2; // 0x920)
    volatile uint32_t conf_mii1_txd1; // 0x924)
    volatile uint32_t conf_mii1_txd0; // 0x928)
    volatile uint32_t conf_mii1_txclk; // 0x92c)
    volatile uint32_t conf_mii1_rxclk; // 0x930)
    volatile uint32_t conf_mii1_rxd3; // 0x934)
    volatile uint32_t conf_mii1_rxd2; // 0x938)
    volatile uint32_t conf_mii1_rxd1; // 0x93c)
    volatile uint32_t conf_mii1_rxd0; // 0x940)
    volatile uint32_t conf_rmii1_refclk; // 0x944)
    volatile uint32_t conf_mdio_data; // 0x948)
    volatile uint32_t conf_mdio_clk; // 0x94c)
    volatile uint32_t conf_spi0_sclk; // 0x950)
    volatile uint32_t conf_spi0_d0; // 0x954)
    volatile uint32_t conf_spi0_d1; // 0x958)
    volatile uint32_t conf_spi0_cs0; // 0x95c)
    volatile uint32_t conf_spi0_cs1; // 0x960)
    volatile uint32_t conf_ecap0_in_pwm0_out; // 0x964)
    struct {
        volatile uint32_t ctsn;
        volatile uint32_t rtsn;
        volatile uint32_t rxd;
        volatile uint32_t txd;
    } conf_uart[2]; // 0x968-0x988
    volatile uint32_t conf_i2c0_sda; // 0x988)
    volatile uint32_t conf_i2c0_scl; // 0x98c)
    volatile uint32_t conf_mcasp0_aclkx; // 0x990)
    volatile uint32_t conf_mcasp0_fsx; // 0x994)
    volatile uint32_t conf_mcasp0_axr0; // 0x998)
    volatile uint32_t conf_mcasp0_ahclkr; // 0x99c)
    volatile uint32_t conf_mcasp0_aclkr; // 0x9a0)
    volatile uint32_t conf_mcasp0_fsr; // 0x9a4)
    volatile uint32_t conf_mcasp0_axr1; // 0x9a8)
    volatile uint32_t conf_mcasp0_ahclkx; // 0x9ac)
    volatile uint32_t conf_xdma_event_intr[2]; // 0x9b0-0x9B8
    volatile uint32_t conf_nresetin_out; // 0x9b8)
    volatile uint32_t conf_porz; // 0x9bc)
    volatile uint32_t conf_nnmi; // 0x9c0)
    char pad23[12];
    volatile uint32_t conf_tms; // 0x9d0)
    volatile uint32_t conf_tdi; // 0x9d4)
    volatile uint32_t conf_tdo; // 0x9d8)
    volatile uint32_t conf_tck; // 0x9dc)
    volatile uint32_t conf_ntrst; // 0x9e0)
    volatile uint32_t conf_emu[2]; // 0x9e4-0x9EC
    char pad24[12];
    volatile uint32_t conf_rtc_porz; // 0x9f8)
    volatile uint32_t conf_pmic_power_en; // 0x9fc)
    volatile uint32_t conf_ext_wakeup; // 0xa00)
    char pad25[0x18];
    volatile uint32_t conf_usb0_drvvbus; // 0xa1c
    char pad26[0x14];
    volatile uint32_t conf_usb1_drvvbus; // 0xa34
    char pad27[0x3C8];
    volatile uint32_t cqdetect_status; // 0xe00)
    volatile uint32_t ddr_io_ctrl; // 0xe04)
    char pad28[4];
    volatile uint32_t vtp_ctrl; // 0xe0c)
    char pad29[4];
    volatile uint32_t vref_ctrl; // 0xe14)
    char pad30[0x178];
    volatile uint32_t tpcc_evt_mux_0_3; // 0xf90)
    volatile uint32_t tpcc_evt_mux_4_7; // 0xf94)
    volatile uint32_t tpcc_evt_mux_8_11; // 0xf98)
    volatile uint32_t tpcc_evt_mux_12_15; // 0xf9c)
    volatile uint32_t tpcc_evt_mux_16_19; // 0xfa0)
    volatile uint32_t tpcc_evt_mux_20_23; // 0xfa4)
    volatile uint32_t tpcc_evt_mux_24_27; // 0xfa8)
    volatile uint32_t tpcc_evt_mux_28_31; // 0xfac)
    volatile uint32_t tpcc_evt_mux_32_35; // 0xfb0)
    volatile uint32_t tpcc_evt_mux_36_39; // 0xfb4)
    volatile uint32_t tpcc_evt_mux_40_43; // 0xfb8)
    volatile uint32_t tpcc_evt_mux_44_47; // 0xfbc)
    volatile uint32_t tpcc_evt_mux_48_51; // 0xfc0)
    volatile uint32_t tpcc_evt_mux_52_55; // 0xfc4)
    volatile uint32_t tpcc_evt_mux_56_59; // 0xfc8)
    volatile uint32_t tpcc_evt_mux_60_63; // 0xfcc)
    volatile uint32_t timer_evt_capt; // 0xfd0)
    volatile uint32_t ecap_evt_capt; // 0xfd4)
    volatile uint32_t adc_evt_capt; // 0xfd8)
    char pad31[0x24];
    volatile uint32_t reset_iso; // 0x1000)
    char pad32[0x314];
    volatile uint32_t dpll_pwr_sw_ctrl; // 0x1318
    volatile uint32_t ddr_cke_ctrl; // 0x131c)
    volatile uint32_t sma2; // 0x1320
    volatile uint32_t m3_txev_eoi; // 0x1324)
    volatile uint32_t ipc_msg_reg[8]; // 0x1328-0x1348
    char pad33[0xBC];
    volatile uint32_t ddr_cmd_ioctrl[3]; // 0x1404-0x1410
    char pad34[0x30];
    volatile uint32_t ddr_data_ioctrl[2]; // 0x1440-0x1448
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
    volatile uint32_t LCR; // 0x0C line control register
    char pad1[0x10];
    volatile uint32_t MDR1; // 0x20 mode definition register 1
    volatile uint32_t MDR2; // 0x24 mode definition register 2
    char pad2[0x18];
    volatile uint32_t SCR; // 0x40 supplementary control register
    volatile uint32_t SSR; // 0x44 supplementary status register
    char pad3[0x8];
    volatile uint32_t MVR; // 0x50 module version register
    volatile uint32_t SYSC; // 0x54 system configuration register
    volatile uint32_t SYSS; // 0x58 system status register
    volatile uint32_t WER; // 0x5C wakeup enable register
    char pad4[0x4];
    volatile uint32_t RXFIFO_LVL; // 0x64
    volatile uint32_t TXFIFO_LVL; // 0x68
    volatile uint32_t IER2; // 0x6C interrupt enable register 2
    volatile uint32_t ISR2; // 0x70 interrupt status register 2
    volatile uint32_t FREQ_SEL; // 0x74
    char pad5[0x8];
    volatile uint32_t MDR3; // 0x80 mode definition register 3
    volatile uint32_t TX_DMA_THRESHOLD; // 0x84
};

static_assert(sizeof(struct HW_UART) == 0x88, "padding");
extern struct HW_UART HW_UART_4;

struct HW_UART_CFGA {
    volatile uint32_t DLL; // 0x00 divisor latches low
    volatile uint32_t DLH; // 0x04 divisor latches high
    volatile uint32_t IIR_FCR; // 0x08 interrupt identification register & FIFO control register
    char pad1[0x4];
    volatile uint32_t MCR; // 0x10 modem control register
    volatile uint32_t LSR; // 0x14 line status register
    volatile uint32_t MSR_TCR; // 0x18 modem status register & transmission control register
    volatile uint32_t SPR_TLR; // 0x1C scratchpad register & trigger level register
    char pad2[0x18];
    volatile uint32_t UASR; // 0x38 UART autobauding status register
};

static_assert(sizeof(struct HW_UART_CFGA) == 0x3C, "padding");
extern struct HW_UART_CFGA HW_UART_4_CFGA;

struct HW_UART_CFGB {
    volatile uint32_t DLL; // 0x00 divisor latches low
    volatile uint32_t DLH; // 0x04 divisor latches high
    volatile uint32_t EFR; // 0x08 enhanced feature register
    char pad1[0x4];
    volatile uint32_t XON1_ADDR; // 0x10 XON1
    volatile uint32_t XON2_ADDR; // 0x14 XON2
    volatile uint32_t XOFF1_TCR; // 0x18 XOFF1 & transmission control register
    volatile uint32_t XOFF2_TLR; // 0x1C XOFF2 & trigger level register
    char pad2[0x18];
    volatile uint32_t UASR; // 0x38 UART autobauding status register
};

static_assert(sizeof(struct HW_UART_CFGB) == 0x3C, "padding");
extern struct HW_UART_CFGB HW_UART_4_CFGB;

struct HW_UART_OP {
    volatile uint32_t RHR_THR; // 0x00 receive/transmit holding register
    volatile uint32_t IER; // 0x04 interrupt enable register
    volatile uint32_t IIR_FCR; // 0x08 interrupt identification register & FIFO control register
    char pad1[0x4];
    volatile uint32_t MCR; // 0x10 modem control register
    volatile uint32_t LSR; // 0x14 line status register
    volatile uint32_t MSR_TCR; // 0x18 modem status register & transmission control register
    volatile uint32_t SPR_TLR; // 0x1C scratchpad register & trigger level register
};

static_assert(sizeof(struct HW_UART_OP) == 0x20, "padding");
extern struct HW_UART_OP HW_UART_4_OP;

#define HW_ALE_CONTROL_EN (1 << 31)
#define HW_ALE_CONTROL_CLEAR (1 << 30)
#define HW_ALE_CONTROL_BYPASS (1 << 4)

#define HW_ALE_PORT_FORWARD (3 << 0)

struct HW_CPSW_ALE {
    volatile uint32_t IDVER; // 0x0
    char pad1[4];
    volatile uint32_t CONTROL; // 0x8
    char pad2[4];
    volatile uint32_t PRESCALE; // 0x10
    char pad3[4];
    volatile uint32_t UNKNOWN_VLAN; // 0x18
    char pad4[4];
    volatile uint32_t TBLCTL; // 0x20
    char pad5[0x10];
    volatile uint32_t TBLW_2; // 0x34
    volatile uint32_t TBLW_1; // 0x38
    volatile uint32_t TBLW_0; // 0x3C
    volatile uint32_t PORTCTL[6]; // 0x40-0x58
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

struct HW_CPSW_CPDMA {
    volatile uint32_t TX_IDVER; // 0x0
    volatile uint32_t TX_CONTROL; // 0x4
    volatile uint32_t TX_TEARDOWN; // 0x8
    char pad1[4];
    volatile uint32_t RX_IDVER; // 0x10
    volatile uint32_t RX_CONTROL; // 0x14
    volatile uint32_t RX_TEARDOWN; // 0x18
    volatile uint32_t CPDMA_SOFT_RESET; // 0x1c
    volatile uint32_t DMACONTROL; // 0x20
    volatile uint32_t DMASTATUS; // 0x24
    volatile uint32_t RX_BUFFER_OFFSET; // 0x28
    volatile uint32_t EMCONTROL; // 0x2c
    volatile uint32_t TX_PRI_RATE[8]; // 0x30-0x50
    char pad2[0x30];
    volatile uint32_t TX_INTSTAT_RAW; // 0x80
    volatile uint32_t TX_INTSTAT_MASKED; // 0x84
    volatile uint32_t TX_INTMASK_SET; // 0x88
    volatile uint32_t TX_INTMASK_CLEAR; // 0x8c
    volatile uint32_t CPDMA_IN_VECTOR; // 0x90
    volatile uint32_t CPDMA_EOI_VECTOR; // 0x94
    char pad3[8];
    volatile uint32_t RX_INTSTAT_RAW; // 0xa0
    volatile uint32_t RX_INTSTAT_MASKED; // 0xa4
    volatile uint32_t RX_INTMASK_SET; // 0xa8
    volatile uint32_t RX_INTMASK_CLEAR; // 0xac
    volatile uint32_t DMA_INTSTAT_RAW; // 0xb0
    volatile uint32_t DMA_INTSTAT_MASKED; // 0xb4
    volatile uint32_t DMA_INTMASK_SET; // 0xb8
    volatile uint32_t DMA_INTMASK_CLEAR; // 0xbc
    volatile uint32_t RX_PENDTHRESH[8]; // 0xc0-0xE0
    volatile uint32_t RX_FREEBUFFER[8]; // 0xe0-0x100
};

static_assert(sizeof(struct HW_CPSW_CPDMA) == 0x100, "padding");
extern struct HW_CPSW_CPDMA HW_CPSW_CPDMA;

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

struct eth_frame;

struct hw_buffer_descriptor {
    volatile struct hw_buffer_descriptor *next;
    volatile struct eth_frame *buffer;
    volatile uint32_t offset_buf_len;
    volatile uint32_t flags_pkt_len;
};

extern struct hw_buffer_descriptor HW_BUFFER_DESCRIPTORS[512];
static_assert(sizeof(struct hw_buffer_descriptor) == 16, "padding");
static_assert(sizeof(HW_ETH_BUFFER) == 0x2000, "padding");

struct HW_CPSW_STATERAM {
    volatile struct hw_buffer_descriptor * volatile TX_HDP[8]; // 0-0x20
    volatile struct hw_buffer_descriptor * volatile RX_HDP[8]; // 0x20-0x40
    volatile struct hw_buffer_descriptor * volatile TX_CP[8]; // 0x40-0x60
    volatile struct hw_buffer_descriptor * volatile RX_CP[8]; // 0x60-0x80
};

static_assert(sizeof(struct HW_CPSW_STATERAM) == 0x80, "padding");
extern struct HW_CPSW_STATERAM HW_CPSW_STATERAM;

struct HW_CPSW_CPTS {
    volatile uint32_t IDVER; // 0x00
    volatile uint32_t CONTROL; // 0x04
    volatile uint32_t RFTCLK_SEL; // 0x08
    volatile uint32_t TS_PUSH; // 0x0c
    volatile uint32_t TS_LOAD_VAL; // 0x10
    volatile uint32_t TS_LOAD_EN; // 0x14
    char pad1[8];
    volatile uint32_t INTSTAT_RAW; // 0x20
    volatile uint32_t INTSTAT_MASKED; // 0x24
    volatile uint32_t INT_ENABLE; // 0x28
    char pad2[4];
    volatile uint32_t EVENT_POP; // 0x30
    volatile uint32_t EVENT_LOW; // 0x34
    volatile uint32_t EVENT_HIGH; // 0x38
};

static_assert(sizeof(struct HW_CPSW_CPTS) == 0x3C, "padding");
extern struct HW_CPSW_CPTS HW_CPSW_CPTS;

struct HW_CPSW_PORT_0 {
    volatile uint32_t CONTROL; // 0x0
    char pad1[4];
    volatile uint32_t MAX_BLKS; // 0x8
    volatile uint32_t BLK_CNT; // 0xc
    volatile uint32_t TX_IN_CTL; // 0x10
    volatile uint32_t PORT_VLAN; // 0x14
    volatile uint32_t TX_PRI_MAP; // 0x18
    volatile uint32_t CPDMA_TX_PRI_MAP; // 0x1c
    volatile uint32_t CPDMA_RX_CH_MAP; // 0x20
    char pad2[12];
    volatile uint32_t RX_DSCP_PRI_MAP[8]; // 0x30-0x50
};

struct HW_CPSW_PORT {
    volatile uint32_t CONTROL; // 0x0
    char pad1[4];
    volatile uint32_t MAX_BLKS; // 0x8
    volatile uint32_t BLK_CNT; // 0xc
    volatile uint32_t TX_IN_CTL; // 0x10
    volatile uint32_t PORT_VLAN; // 0x14
    volatile uint32_t TX_PRI_MAP; // 0x18
    volatile uint32_t TS_SEQ_MTYPE; // 0x1C
    volatile uint32_t SA_LO; // 0x20
    volatile uint32_t SA_HI; // 0x24
    volatile uint32_t SEND_PERCENT; // 0x28
    char pad2[4];
    volatile uint32_t RX_DSCP_PRI_MAP[8]; // 0x30-0x50
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
    volatile uint32_t IDVER; // 0x0
    volatile uint32_t MACCONTROL; // 0x4
    volatile uint32_t MACSTATUS; // 0x8
    volatile uint32_t SOFT_RESET; // 0xc
    volatile uint32_t RX_MAXLEN; // 0x10
    volatile uint32_t BOFFTEST; // 0x14
    volatile uint32_t RX_PAUSE; // 0x18
    volatile uint32_t TX_PAUSE; // 0x1c
    volatile uint32_t EMCONTROL; // 0x20
    volatile uint32_t RX_PRI_MAP; // 0x24
    volatile uint32_t TX_GAP; // 0x28
};

static_assert(sizeof(struct HW_CPSW_SL) == 0x2C, "padding");
extern struct HW_CPSW_SL HW_CPSW_SL_1;
extern struct HW_CPSW_SL HW_CPSW_SL_2;

#define HW_SS_SOFT_RESET (1 << 0)
#define HW_SS_STAT_EN_PORT_0 (1 << 0)
#define HW_SS_STAT_EN_PORT_1 (1 << 1)
#define HW_SS_STAT_EN_PORT_2 (1 << 2)

struct HW_CPSW_SS {
    volatile uint32_t ID_VER; // 0x0
    volatile uint32_t CONTROL; // 0x4
    volatile uint32_t SOFT_RESET; // 0x8
    volatile uint32_t STAT_PORT_EN; // 0xc
    volatile uint32_t PTYPE; // 0x10
    volatile uint32_t SOFT_IDLE; // 0x14
    volatile uint32_t THRU_RATE; // 0x18
    volatile uint32_t GAP_THRESH; // 0x1c
    volatile uint32_t TX_START_WDS; // 0x20
    volatile uint32_t FLOW_CONTROL; // 0x24
    volatile uint32_t VLAN_LTYPE; // 0x28
    volatile uint32_t TS_LTYPE; // 0x2c
    volatile uint32_t DLR_LTYPE; // 0x30
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
    volatile uint32_t IDVER; // 0x0
    volatile uint32_t SOFT_RESET; // 0x04
    volatile uint32_t CONTROL; // 0x08
    volatile uint32_t INT_CONTROL; // 0x0c
    struct {
        volatile uint32_t RX_THRESH;
        volatile uint32_t RX;
        volatile uint32_t TX;
        volatile uint32_t MISC;
    } PORT_INT_ENABLE[3]; // 0x10-0x40
    struct {
        volatile uint32_t RX_THRESH;
        volatile uint32_t RX;
        volatile uint32_t TX;
        volatile uint32_t MISC;
    } PORT_INT_STATUS[3]; // 0x40-0x70
    struct {
        volatile uint32_t RX;
        volatile uint32_t TX;
    } PORT_INT_MAX[3]; // 0x70-0x88
    volatile uint32_t RGMII_CTL; // 0x88
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
    volatile uint32_t REVID; // 0x0
    volatile uint32_t CONTROL; // 0x4
    volatile uint32_t ALIVE; // 0x8
    volatile uint32_t LINK; // 0xC
    volatile uint32_t LINKINTRAW; // 0x10
    volatile uint32_t LINKINTMASKED; // 0x14
    char pad1[8];
    volatile uint32_t USERINTRAW; // 0x20
    volatile uint32_t USERINTMASKED; // 0x24
    volatile uint32_t USERINTMASKSET; // 0x28
    volatile uint32_t USERINTMASKCLEAR; // 0x2C
    char pad2[0x50];
    volatile uint32_t USERACCESS0; // 0x80
    volatile uint32_t USERPHYSEL0; // 0x84
    volatile uint32_t USERACCESS1; // 0x88
    volatile uint32_t USERPHYSEL1; // 0x8C
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
    volatile uint32_t REVISION; // 0x0
    char pad[12];
    volatile uint32_t SYSCONFIG; // 0x10
    volatile uint32_t SYSSTATUS; // 0x14
    char pad2[0x28];
    volatile uint32_t SIR_IRQ; // 0x40
    volatile uint32_t SIR_FIQ; // 0x44
    volatile uint32_t CONTROL; // 0x48
    volatile uint32_t PROTECTION; // 0x4c
    volatile uint32_t IDLE; // 0x50
    char pad3[12];
    volatile uint32_t IRQ_PRIORITY; // 0x60
    volatile uint32_t FIQ_PRIORITY; // 0x64
    volatile uint32_t THRESHOLD; // 0x68
    char pad4[0x14];
    struct {
        volatile uint32_t ITR; // 0x00
        volatile uint32_t MIR; // 0x04
        volatile uint32_t MIR_CLEAR; // 0x08
        volatile uint32_t MIR_SET; // 0x0C
        volatile uint32_t ISR_SET; // 0x10
        volatile uint32_t ISR_CLEAR; // 0x14
        volatile uint32_t PENDING_IRQ; // 0x18
        volatile uint32_t PENDING_FIQ; // 0x1C
    } BANK[4]; // 0x80-0x100

    volatile uint32_t ILR[128]; // 0x100-0x300
};

static_assert(sizeof(struct HW_INTC) == 0x300, "padding");
extern struct HW_INTC HW_INTC;

struct HW_GPIO {
    volatile uint32_t REVISION; // 0x0
    char pad1[12];
    volatile uint32_t SYSCONFIG; // 0x10
    char pad2[12];
	volatile uint32_t EOI; // 0x20
    volatile uint32_t IRQSTATUS_RAW[2]; // 0x24-0x2C
    volatile uint32_t IRQSTATUS[2]; // 0x2C-0x34
    volatile uint32_t IRQSTATUS_SET[2]; // 0x34-0x3C
    volatile uint32_t IRQSTATUS_CLR[2]; // 0x3C-0x44
    volatile uint32_t IRQWAKEN[2]; // 0x44-0x4C
    char pad3[0xC8];
    volatile uint32_t SYSSTATUS; // 0x114
    char pad4[0x18];
    volatile uint32_t CTRL; // 0x130
    volatile uint32_t INPUT_EN; // 0x134
    volatile uint32_t DATAIN; // 0x138
    volatile uint32_t DATAOUT; // 0x13C
    volatile uint32_t LEVELDETECT[2]; // 0x140-0x148
    volatile uint32_t RISINGDETECT; // 0x148
    volatile uint32_t FALLINGDETECT; // 0x14C
    volatile uint32_t DEBOUNCENABLE; // 0x150
    volatile uint32_t DEBOUNCINGTIME; // 0x154
    char pad5[0x38];
    volatile uint32_t CLEARDATAOUT; // 0x190
    volatile uint32_t SETDATAOUT; // 0x194
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
    volatile uint32_t TIDR; // 0x0
    char pad1[12];
    volatile uint32_t TIOCP_CFG; // 0x10
    volatile uint32_t TISTAT; // 0x14
    volatile uint32_t TISR; // 0x18
    volatile uint32_t TIER; // 0x1c
    volatile uint32_t TWER; // 0x20
    volatile uint32_t TCLR; // 0x24
    volatile uint32_t TCRR; // 0x28
    volatile uint32_t TLDR; // 0x2c
    volatile uint32_t TTGR; // 0x30
    volatile uint32_t TWPS; // 0x34
    volatile uint32_t TMAR; // 0x38
    volatile uint32_t TCAR1; // 0x3C
    volatile uint32_t TSICR; // 0x40
    volatile uint32_t TCAR2; // 0x44
    volatile uint32_t TPIR; // 0x48
    volatile uint32_t TNIR; // 0x4c
    volatile uint32_t TCVR; // 0x50
    volatile uint32_t TOCR; // 0x54
    volatile uint32_t TOWR; // 0x58
};

static_assert(sizeof(struct HW_DMTIMER_1MS) == 0x5C, "padding");
extern struct HW_DMTIMER_1MS HW_DMTIMER_1MS;


#include <stdint.h>
#include <assert.h>
#include <stddef.h>

static int g_foo = 1;
static int g_foo2;

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
	volatile uint32_t L3_instruction;
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
	volatile uint32_t OCPWP_L3_status;
	volatile uint32_t OCPWP;
	char pad[12];
	volatile uint32_t PRU_ICSS_status;
	volatile uint32_t CPSW;
	volatile uint32_t LCDC_status;
	volatile uint32_t CLKDIV32K;
	volatile uint32_t CLK_24MHZ;
};

static_assert(sizeof(struct HW_CM_PER) == 0x154, "padding");
extern struct HW_CM_PER HW_CM_PER;

struct HW_CM_WAKEUP {
	volatile uint32_t CLKSTCTRL; //   (0x0)
	volatile uint32_t CONTROL_CLKCTRL; //   (0x4)
	volatile uint32_t GPIO0_CLKCTRL; //   (0x8)
	volatile uint32_t L4WKUP_CLKCTRL; //   (0xc)
	volatile uint32_t TIMER0_CLKCTRL; //   (0x10)
	volatile uint32_t DEBUGSS_CLKCTRL; //   (0x14)
	volatile uint32_t CM_L3_AON_CLKSTCTRL; //   (0x18)
	volatile uint32_t CM_AUTOIDLE_DPLL_MPU; //   (0x1c)
	volatile uint32_t CM_IDLEST_DPLL_MPU; //   (0x20)
	volatile uint32_t CM_SSC_DELTAMSTEP_DPLL_MPU; //   (0x24)
	volatile uint32_t CM_SSC_MODFREQDIV_DPLL_MPU; //   (0x28)
	volatile uint32_t CM_CLKSEL_DPLL_MPU; //   (0x2c)
	volatile uint32_t CM_AUTOIDLE_DPLL_DDR; //   (0x30)
	volatile uint32_t CM_IDLEST_DPLL_DDR; //   (0x34)
	volatile uint32_t CM_SSC_DELTAMSTEP_DPLL_DDR; //   (0x38)
	volatile uint32_t CM_SSC_MODFREQDIV_DPLL_DDR; //   (0x3c)
	volatile uint32_t CM_CLKSEL_DPLL_DDR; //   (0x40)
	volatile uint32_t CM_AUTOIDLE_DPLL_DISP; //   (0x44)
	volatile uint32_t CM_IDLEST_DPLL_DISP; //   (0x48)
	volatile uint32_t CM_SSC_DELTAMSTEP_DPLL_DISP; //   (0x4c)
	volatile uint32_t CM_SSC_MODFREQDIV_DPLL_DISP; //   (0x50)
	volatile uint32_t CM_CLKSEL_DPLL_DISP; //   (0x54)
	volatile uint32_t CM_AUTOIDLE_DPLL_CORE; //   (0x58)
	volatile uint32_t CM_IDLEST_DPLL_CORE; //   (0x5c)
	volatile uint32_t CM_SSC_DELTAMSTEP_DPLL_CORE; //   (0x60)
	volatile uint32_t CM_SSC_MODFREQDIV_DPLL_CORE; //   (0x64)
	volatile uint32_t CM_CLKSEL_DPLL_CORE; //   (0x68)
	volatile uint32_t CM_AUTOIDLE_DPLL_PER; //   (0x6c)
	volatile uint32_t CM_IDLEST_DPLL_PER; //   (0x70)
	volatile uint32_t CM_SSC_DELTAMSTEP_DPLL_PER; //   (0x74)
	volatile uint32_t CM_SSC_MODFREQDIV_DPLL_PER; //   (0x78)
	volatile uint32_t CM_CLKDCOLDO_DPLL_PER; //   (0x7c)
	volatile uint32_t CM_DIV_M4_DPLL_CORE; //   (0x80)
	volatile uint32_t CM_DIV_M5_DPLL_CORE; //   (0x84)
	volatile uint32_t CM_CLKMODE_DPLL_MPU; //   (0x88)
	volatile uint32_t CM_CLKMODE_DPLL_PER; //   (0x8c)
	volatile uint32_t CM_CLKMODE_DPLL_CORE; //   (0x90)
	volatile uint32_t CM_CLKMODE_DPLL_DDR; //   (0x94)
	volatile uint32_t CM_CLKMODE_DPLL_DISP; //   (0x98)
	volatile uint32_t CM_CLKSEL_DPLL_PERIPH; //   (0x9c)
	volatile uint32_t CM_DIV_M2_DPLL_DDR; //   (0xa0)
	volatile uint32_t CM_DIV_M2_DPLL_DISP; //   (0xa4)
	volatile uint32_t CM_DIV_M2_DPLL_MPU; //   (0xa8)
	volatile uint32_t CM_DIV_M2_DPLL_PER; //   (0xac)
	volatile uint32_t WKUP_M3_CLKCTRL; //   (0xb0)
	volatile uint32_t UART0_CLKCTRL; //   (0xb4)
	volatile uint32_t I2C0_CLKCTRL; //   (0xb8)
	volatile uint32_t ADC_TSC_CLKCTRL; //   (0xbc)
	volatile uint32_t SMARTREFLEX0_CLKCTRL; //   (0xc0)
	volatile uint32_t TIMER1_CLKCTRL; //   (0xc4)
	volatile uint32_t SMARTREFLEX1_CLKCTRL; //   (0xc8)
	volatile uint32_t CM_L4_WKUP_AON_CLKSTCTRL; //   (0xcc)
	volatile uint32_t WDT0_CLKCTRL; //   (0xd0)
	volatile uint32_t WDT1_CLKCTRL; //   (0xd4)
	volatile uint32_t CM_DIV_M6_DPLL_CORE; //   (0xd8)
};

static_assert(sizeof(struct HW_CM_WAKEUP) == 0xDC, "padding");
extern struct HW_CM_WAKEUP HW_CM_WKUP;

struct HW_GPIO {
	volatile uint32_t revision;
	char pad1[12];
	volatile uint32_t sysconfig;
	char pad2[12];
	volatile uint32_t irqstatus_raw[2];
	volatile uint32_t irqstatus[2];
	volatile uint32_t irqstatus_set[2];
	volatile uint32_t irqstatus_clear[2];
	volatile uint32_t irqwaken[2];
	char pad3[0xCC];
	volatile uint32_t sysstatus;
	char pad4[0x18];
	volatile uint32_t control;
	volatile uint32_t output_enable;
	volatile uint32_t data_in;
	volatile uint32_t data_out;
	volatile uint32_t level_detect[2];
	volatile uint32_t rising_detect;
	volatile uint32_t falling_detect;
	volatile uint32_t debounce_enable;
	volatile uint32_t debounce_time;
	char pad5[0x38];
	volatile uint32_t clear_data_out;
	volatile uint32_t set_data_out;
};

static_assert(sizeof(struct HW_GPIO) == 0x198, "padding");
extern struct HW_GPIO HW_GPIO0;

#define HWREG(x) (*((volatile unsigned int *)(x)))

extern void UARTConsoleInit(void);
extern void UARTConsolePutc(unsigned char data);

static void print(const char *str) {
	while (*str) {
		UARTConsolePutc(*str);
		str++;
	}
}

void led_wobble(void) {
	HW_GPIO0.set_data_out = 1 << 23;

    for (;;) {
        for (int i = 0; i < 1000000; i++) {

        }
		HW_GPIO0.data_out = ~HW_GPIO0.data_out;
    }
}

int main(void) {
    // boot-button is 2.8
    // leds are 1.21-24

	UARTConsoleInit();
	print("hello world\r\n");
	led_wobble();
    return 0;
}
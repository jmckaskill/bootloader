#include "am335x.h"
#include "../tinyprintf/tinyprintf.h"
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include "endian.h"

extern void interrupt();
extern void setup();

void led_wobble(void) {
	HW_GPIO_1.SETDATAOUT = 1 << 23;

    for (;;) {
        for (int i = 0; i < 1000000; i++) {

        }
		HW_GPIO_1.DATAOUT = ~HW_GPIO_1.DATAOUT;
    }
}

static void enable_module(volatile uint32_t *ctrl) {
	*ctrl |= HW_CM_MODULEMODE_ENABLE;
	while ((*ctrl & HW_CM_MODULEMODE_MASK) != HW_CM_MODULEMODE_ENABLE) {
	}
}

static void sw_wakeup(volatile uint32_t *status) {
	*status |= HW_CM_CLKTRCTRL_SW_WKUP;
	while ((*status & HW_CM_CLKTRCTRL_MASK) != HW_CM_CLKTRCTRL_SW_WKUP){
	}
}

static void wait_for_functional(volatile uint32_t *ctrl) {
	while ((*ctrl & HW_CM_IDLEST_MASK) != HW_CM_IDLEST_FUNCTIONAL) {
	}
}

static void enable_interconnects() {
	// enable L3 & L3S clocks in the peripheral power domain
	enable_module(&HW_CM_PER.L3);
	enable_module(&HW_CM_PER.L3_instr);
	sw_wakeup(&HW_CM_PER.L3_status);
	sw_wakeup(&HW_CM_PER.OCPWP_status);
	sw_wakeup(&HW_CM_PER.L3S_status);
	wait_for_functional(&HW_CM_PER.L3);
	wait_for_functional(&HW_CM_PER.L3_instr);
	while (!(HW_CM_PER.L3_status & HW_CM_L3_GCLK)) {}
	while (!(HW_CM_PER.OCPWP_status & HW_CM_OCPWP_L3_GCLK)) {}
	while (!(HW_CM_PER.L3S_status & HW_CM_L3S_GCLK)) {}

	// enable L3 clocks in the wakeup power domain
	enable_module(&HW_CM_WAKEUP.CONTROL_CLKCTRL);
	sw_wakeup(&HW_CM_WAKEUP.CLKSTCTRL);
	sw_wakeup(&HW_CM_WAKEUP.L3_AON_CLKSTCTRL);
	wait_for_functional(&HW_CM_WAKEUP.CONTROL_CLKCTRL);
	while (!(HW_CM_WAKEUP.L3_AON_CLKSTCTRL & HW_CM_L3_AON_GCLK)) {}

	// enable L4 clocks in the wakeup power domain
	// no need to enable the L4 wakeup module - it can't be disabled
	wait_for_functional(&HW_CM_WAKEUP.L4WKUP_CLKCTRL);
	while (!(HW_CM_WAKEUP.CLKSTCTRL & HW_CM_WAKEUP_L4_GCLK)) {}
	while (!(HW_CM_WAKEUP.L4_WKUP_AON_CLKSTCTRL & HW_CM_L4_AON_GCLK)) {}
	//while (!(HW_CM_PER.OCPWP_status & HW_CM_OCPWP_L4_GCLK)) {}

	// enable L4LS clocks in the peripheral power domain
	enable_module(&HW_CM_PER.L4LS);
	sw_wakeup(&HW_CM_PER.L4LS_status);
	wait_for_functional(&HW_CM_PER.L4LS);
	while (!(HW_CM_PER.L4LS_status & HW_CM_L4LS_GCLK)) {}

	// enable L4HS clocks in the peripheral power domain
	enable_module(&HW_CM_PER.L4HS);
	sw_wakeup(&HW_CM_PER.L4HS_status);
	wait_for_functional(&HW_CM_PER.L4HS);
	while (!(HW_CM_PER.L4HS_status & HW_CM_L4HS_GCLK)) {}
}

enum uart_tx_select {
	TX_UART4,
	TX_UART5,
};

static void set_tx_uart(enum uart_tx_select tx) {
	enable_module(&HW_CM_WAKEUP.GPIO0_CLKCTRL);
	wait_for_functional(&HW_CM_WAKEUP.GPIO0_CLKCTRL);

	HW_CONTROL.conf_gpmc_ad[11] = HW_CONTROL_MUXMODE_7 | HW_CONTROL_DISABLE_PULL;
	HW_GPIO_0.INPUT_EN &= ~(1 << 27);

	switch (tx) {
	case TX_UART4:
		HW_GPIO_0.CLEARDATAOUT |= (1 << 27);
		break;
	case TX_UART5:
		HW_GPIO_0.SETDATAOUT |= (1 << 27);
		break;
	}
}

static void enable_uart4() {
	set_tx_uart(TX_UART4);

	HW_CONTROL.conf_uart[0].rtsn = HW_CONTROL_MUXMODE_1 | HW_CONTROL_PULL_UP;
	HW_CONTROL.conf_uart[0].ctsn = HW_CONTROL_MUXMODE_1 | HW_CONTROL_PULL_UP | HW_CONTROL_RX_ACTIVE;

	// enable the module
	enable_module(&HW_CM_PER.UART4);
	wait_for_functional(&HW_CM_PER.UART4);
	while (!(HW_CM_PER.L4LS_status & HW_CM_L4LS_UART_GFCLK)) {}

	// reset the module
	HW_UART_4.SYSC |= HW_UART_SYSC_SOFTRESET;
	while (!(HW_UART_4.SYSS & HW_UART_SYSS_RESETDONE)) {}

	// enable the output to 115200 8N1
	HW_UART_4.LCR = HW_UART_LCR_CFGB;
	HW_UART_4_CFGB.DLH = 0x00;
	HW_UART_4_CFGB.DLL = 0x1A;
	HW_UART_4.LCR = HW_UART_LCR_LENGTH_8 | HW_UART_LCR_STOP_1;
	//HW_UART_4_OP.MCR |= HW_UART_MCR_LOOPBACK_EN;
	HW_UART_4.MDR1 = HW_UART_MDR1_MODE_UART16X;

	// at this point we are in operational mode
}

static void enable_gpio3() {
	enable_module(&HW_CM_PER.GPIO3);
	wait_for_functional(&HW_CM_PER.GPIO3);
}

static void set_switch_to_ground() {
	HW_CONTROL.conf_mcasp0_fsr = HW_CONTROL_MUXMODE_7 | HW_CONTROL_DISABLE_PULL;
	HW_GPIO_3.INPUT_EN &= ~(1 << 19);
	HW_GPIO_3.SETDATAOUT |= (1 << 19);
}

#define MS_TO_TICKS(ms) ((ms)*32768/1000)

static uint32_t tick_count() {
	return HW_DMTIMER_1MS.TCRR;
}

static void set_sleep(uint32_t target) {
	HW_DMTIMER_1MS.TMAR = target;
}

static void enable_interrupt(enum hw_interrupt idx) {
	HW_INTC.BANK[idx >> 5].MIR_CLEAR |= 1 << ((unsigned)idx & 0x1F);
}

static void enable_dmtimer_1ms() {
	enable_module(&HW_CM_WAKEUP.TIMER1_CLKCTRL);
	wait_for_functional(&HW_CM_WAKEUP.TIMER1_CLKCTRL);
	while (!(HW_CM_WAKEUP.CLKSTCTRL & HW_CM_WAKEUP_TIMER1_GCLK)) {}

	HW_DMTIMER_1MS.TIOCP_CFG |= HW_1MS_CFG_SOFT_RESET;
	while (!(HW_DMTIMER_1MS.TISTAT & HW_1MS_TISTAT_RESET_DONE)) {}

	// Setup the clock to use the 32k external clock with no division
	HW_CM_DPLL.CLKSEL_TIMER1MS_CLK = HW_CLKSEL_TIMER1MS_32K_EXT;

	// enable the interrupt
	HW_DMTIMER_1MS.TIER |= HW_1MS_INT_COMPARE;
	enable_interrupt(HW_INT_TINT1_1MS);

	set_sleep(tick_count() + MS_TO_TICKS(1000));

	// enable the timer in compare mode. this will trigger an interrupt whenever
	// the internal timer matches the comparison register
	HW_DMTIMER_1MS.TCLR |= HW_1MS_TCLR_COMPARE_EN | HW_1MS_TCLR_START;
}

static int read_mdio(int phy, int key) {
	HW_MDIO.USERACCESS0 = HW_MDIO_READ(phy, key);
	unsigned reg;
	do {
		reg = HW_MDIO.USERACCESS0;
	} while (reg & HW_MDIO_GO);

	if (reg & HW_MDIO_ACK) {
		return HW_MDIO_VALUE(reg);
	} else {
		return -1;
	}
}

static void write_mdio(int phy, int key, int val) {
	HW_MDIO.USERACCESS0 = HW_MDIO_WRITE(phy, key, val);
	while (!(HW_MDIO.USERACCESS0 & HW_MDIO_GO)) {}
}


static const char g_testmsg[] =
"\xff\xff\xff\xff\xff\xff\xb0\xd5\xcc\x6e\x63\x88\x08\x00\x45\x00" \
"\x01\x88\x01\x00\x00\x00\x40\x11\x78\x66\x00\x00\x00\x00\xff\xff" \
"\xff\xff\x00\x44\x00\x43\x01\x74\x11\xa0\x01\x01\x06\x00\x00\x00" \
"\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\xb0\xd5\xcc\x6e\x63\x88\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x63\x82\x53\x63\x3c\x0a\x41\x4d\x33\x33" \
"\x35\x78\x20\x52\x4f\x4d\x3d\x51\x05\x01\x05\x01\x81\x40\x07\x03" \
"\x13\x02\x01\x00\x12\x15\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\x21\x01\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x15\x09" \
"\x01\x86\x3d\x37\x8a\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
"\x00\x00\x00\x00\x00\x00";

#define ETH_OFFSET (2 << 16)
#define ETH_IP6 0x86DD
#define IP6_TCP 6
#define IP6_UDP 17
#define IP6_ICMP 58
#define IP6_DEFAULT_HEADER 0x60000000
#define IP6_DEFAULT_HOP_LIMIT 255

struct udp_frame {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t length;
	uint16_t checksum;
	char data[1];
};

struct tcp_frame {
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t seq_num;
	uint32_t ack_num;
	uint16_t flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent;
	char data[1];
};

#define ICMP6_NEIGHBOR_SOLICITATION 135
#define ICMP6_NEIGHBOR_ADVERTISEMENT 136

#define ALL_NODES_0 0xFF020000
#define ALL_NODES_1 0
#define ALL_NODES_2 0
#define ALL_NODES_3 0
#define ALL_NODES_4 1

#define ETH_MCAST_HI 0x3333

struct icmp6_frame {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t body;
};

struct eth_frame {
	uint8_t padding[2];
	uint16_t eth_dst_hi;
	uint32_t eth_dst_lo;
	uint16_t eth_src_hi;
	uint32_t eth_src_lo;
	uint16_t eth_type;
	uint32_t ip6_header;
	uint16_t ip6_length;
	uint8_t ip6_next_header;
	uint8_t ip6_hop_limit;
	uint32_t ip6_src[4];
	uint32_t ip6_dst[4];
	char payload[0x600 - 40 - 14 - 2];
};

static_assert(sizeof(struct eth_frame) == 0x600, "padding");

// about 49 KB out of 64KB of the L3 OCMC0 memory
#define RX_FRAME_FIRST 0
#define RX_FRAME_END 0x10
#define TX_FRAME_FIRST 0x10
#define TX_FRAME_END 0x20


extern struct eth_frame g_eth_frames[TX_FRAME_END];

static int g_tx_next;
static struct hw_buffer_descriptor *g_rx_last;

static int g_tx_next; // next frame to be sent
static int g_tx_end; // last frame available to be sent + 1
static struct hw_buffer_descriptor *g_tx_last; // last frame we handed off to the mac

// gives the next index in the circular buffer
#define NEXT_TX_FRAME(idx) (((((idx) - TX_FRAME_FIRST) + 1) & (TX_FRAME_END - TX_FRAME_FIRST)) + TX_FRAME_FIRST)

#if 0
static struct eth_frame *get_next_frame() {
	if (g_tx_next == g_tx_end) {
		return NULL;
	}
	return &g_eth_frames[g_tx_next]
}

static void send_frame(int sz) {
	int idx = g_tx_next;
	struct eth_frame *f = &g_eth_frames[idx];
	struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[idx];

	d->next = NULL;
	d->buffer = f;
	d->offset_buf_len = ETH_OFFSET | sz;
	d->flags_pkt_len = HW_ETH_FULL_PKT | HW_ETH_OWNED_BY_PORT | sz;

	if (!g_tx_last) {
		HW_CPSW_STATERAM.TX_HDP[0] = d;
	} else {
		g_tx_last->next = d;
		g_tx_last = d;
	}
}

static int g_have_sent_initial;

// in network byte order
static uint16_t g_my_mac_hi;
static uint32_t g_my_mac_lo;
static uint32_t g_my_ip[4];

static void send_initial_messages() {
	struct eth_frame *f = get_next_frame();

	f->eth_dst_hi = htons(ETH_MCAST_HI);
	f->eth_dst_lo = htonl(ALL_NODES_4);
	f->eth_src_hi = g_my_mac_hi;
	f->eth_src_lo = g_my_mac_lo;
	f->eth_type = ETH_IP6;
	f->ip6_header = IP6_DEFAULT_HEADER;
	f->ip6_length = 
}
#endif

static void clear_rx_descriptor(struct hw_buffer_descriptor *d) {
	d->next = NULL;
	d->offset_buf_len = HW_ETH_MAX_LEN;
	d->flags_pkt_len = HW_ETH_OWNED_BY_PORT;
}

static void enable_eth() {
	HW_CONTROL.gmii_sel = HW_CONTROL_GMII_1 | HW_CONTROL_GMII_2;

	enable_module(&HW_CM_PER.CPGMAC0);
	sw_wakeup(&HW_CM_PER.CPSW_status);
	wait_for_functional(&HW_CM_PER.CPGMAC0);
	while (!(HW_CM_PER.CPSW_status & HW_CM_CPSW_125MHZ_GCLK)) {}
	while (!(HW_CM_PER.L4HS_status & HW_CM_L4HS_CPSW_5MHZ_GCLK)) {}
	while (!(HW_CM_PER.L4HS_status & HW_CM_L4HS_CPSW_50MHZ_GCLK)) {}
	while (!(HW_CM_PER.L4HS_status & HW_CM_L4HS_CPSW_25MHZ_GCLK)) {}

	// reset the various ethernet subsystems
	// resetting the wrapper also resets the MDIO module
	HW_CPSW_WR.SOFT_RESET = 1;
	HW_CPSW_WR.CONTROL = HW_WR_CONTROL_NO_STANDBY | HW_WR_CONTROL_NO_IDLE;
	
	HW_CPSW_CPDMA.CPDMA_SOFT_RESET = 1;
	while (HW_CPSW_CPDMA.CPDMA_SOFT_RESET) {}
	
	HW_CPSW_SL_1.SOFT_RESET = 1;
	HW_CPSW_SL_2.SOFT_RESET = 1;
	while (HW_CPSW_SL_1.SOFT_RESET) {}
	while (HW_CPSW_SL_2.SOFT_RESET) {}

	HW_CPSW_SS.SOFT_RESET = 1;
	while (HW_CPSW_SS.SOFT_RESET) {}

	// start setting stuff back up
	HW_CPSW_ALE.CONTROL = HW_ALE_CONTROL_CLEAR | HW_ALE_CONTROL_EN;
	HW_CPSW_ALE.PORTCTL[0] = HW_ALE_PORT_FORWARD;
	HW_CPSW_ALE.PORTCTL[1] = HW_ALE_PORT_FORWARD;

	HW_CPSW_PORT_1.SA_LO = HW_CONTROL.mac[0].lo;
	HW_CPSW_PORT_1.SA_HI = HW_CONTROL.mac[0].hi;
	HW_CPSW_PORT_2.SA_LO = HW_CONTROL.mac[1].lo;
	HW_CPSW_PORT_2.SA_HI = HW_CONTROL.mac[1].hi;

	// main CPSW clock is 125 MHz. this gives an MDIO frequency of 2.5 MHz
	// which matches what the ROM does and the max frequency of the PHY
	// this also puts the other fields back to default values
	HW_MDIO.CONTROL = (HW_MDIO.CONTROL &~ HW_MDIO_CLKDIV_MASK) | HW_MDIO_CLKDIV(49);
	HW_MDIO.CONTROL |= HW_MDIO_CONTROL_EN;
	
	// setup the phy
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_SOFT_RESET);
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_AUTO_NEGOTIATION | MDIO_CTL_RESTART_AUTO);

	// setup rx frames
	for (int i = RX_FRAME_FIRST; i < RX_FRAME_END; i++) {
		struct hw_eth_buffer *d = &HW_BUFFER_DESCRIPTORS[i];
		struct eth_frame *f = &g_eth_frames[i];
		h->buffer = f;
		clear_rx_descriptor(d);
		
		if (i > RX_FRAME_FIRST) {
			HW_BUFFER_DESCRIPTORS[i-1].next = h;
		}
	}
	g_rx_last = &HW_BUFFER_DESCRIPTORS[RX_FRAME_END-1];

	HW_CPSW_CPDMA.RX_BUFFER_OFFSET = offsetof(struct eth_frame, eth_dst);
	HW_CPSW_CPDMA.RX_HDP[0] = &HW_BUFFER_DESCRIPTORS[0];

	// setup tx frames
	g_tx_next = TX_FRAME_FIRST;
	g_tx_end = TX_FRAME_END;

	// enable the rx interrupts
	HW_CPSW_WR.INT_CONTROL = HW_WR_INT_PAGE_RX_0;
	HW_CPSW_WR.PORT_INT_ENABLE[0].RX_THRESH = 1;
	HW_CPSW_WR.PORT_INT_ENABLE[0].RX = 1;
	HW_CPSW_WR.PORT_INT_MAX[0].RX = 1; // max 1 interrupt/ms
	HW_CPSW_CPDMA.RX_INTMASK_SET = HW_CPDMA_RX_PEND_0 | HW_CPDMA_RX_THRESH_PEND_0;

	HW_CPSW_CPDMA.RX_PENDTHRESH[0] = 2;
	HW_CPSW_CPDMA.RX_FREEBUFFER[0] = RX_FRAME_END - RX_FRAME_FIRST;

	enable_interrupt(HW_INT_ETH_RX_THRESHOLD);
	enable_interrupt(HW_INT_ETH_RX);

	// enable the port
	HW_CPSW_CPDMA.TX_CONTROL = HW_CPDMA_TX_EN;
	HW_CPSW_CPDMA.RX_CONTROL = HW_CPDMA_RX_EN;

	//g_have_sent_initial = 0;
}

static void wait_for_link() {
	int sts;
	do {
		sts = read_mdio(0, MDIO_BASIC_STATUS);
	} while (!(sts & MDIO_STS_LINK_UP));

	printf("mdio link changed 0x%x %x %x\n", sts, HW_MDIO.LINK, HW_MDIO.ALIVE);
	HW_CPSW_SL_1.MACCONTROL = HW_SL_MACCONTROL_MII_EN | HW_SL_MACCONTROL_FULL_DUPLEX;
	while (!(HW_CPSW_SL_1.MACCONTROL & HW_SL_MACCONTROL_MII_EN)) {}
}

static void debug_putc(void* udata, char ch) {
	while ((HW_UART_4_OP.LSR & HW_UART_LSR_TX_EMPTY) != HW_UART_LSR_TX_EMPTY) {}
	HW_UART_4_OP.RHR_THR = ch;
}

static void process_frame(struct eth_frame *f, int sz) {
	sz -= offsetof(struct eth_frame, u);
	if (sz < 0 || ntohs(f->eth_type) != ETH_IP6)  {
		return;
	}

	printf("have frame from %04x%08x\n", f->eth_src_hi, f->eth_src_lo);
}

void interrupt() {
	int end = 0;
	switch (HW_INTC.SIR_IRQ & HW_INTC_ACTIVE_IRQ_MASK) {
	case HW_INT_TINT1_1MS: {
			unsigned now = tick_count();
			int sts = read_mdio(0, MDIO_BASIC_STATUS);
			printf("timer %u %x\n", now, sts);
#if 0
			if (!g_have_sent_initial) {
				send_initial_messages();
				g_have_sent_initial = 1;
			}
#endif
			// reset the timer interrupt
			HW_DMTIMER_1MS.TISR |= HW_1MS_INT_COMPARE;
			set_sleep(now + MS_TO_TICKS(1000));
		}
		break;
	case HW_INT_ETH_RX:
		end = 1; // fallthrough
	case HW_INT_ETH_RX_THRESHOLD: {
			// process each frame one by one, readding as we go
			struct hw_buffer_descriptor *d = g_rx_next;
			struct hw_buffer_descriptor *last = g_rx_last;

			for (;;) {
				uint32_t flags = d->flags_pkt_len;
				if (flags & HW_ETH_OWNED_BY_PORT) {
					// we've processed all that are ready
					break;
				}
				if (flags & HW_ETH_END_OF_QUEUE) {
					// receiver ran out of packets, reinitiate the queue
					HW_CPSW_STATERAM.RX_HDP[0] = d->next;
				}

				if ((flags & (HW_ETH_FULL_PKT | HW_ETH_ERROR_MASK) == (HW_ETH_FULL_PKT | HW_ETH_NO_ERROR)) {
					// we shouldn't be getting partial frames
					// in case we do, drop them
					int sz = d->offset_buf_len & 0xFFFF;
					if (flags & HW_ETH_PASS_CRC) {
						sz -= 4;
					}
					process_frame(d->buffer, sz);
				}
				
				// readd the frame and move onto the next
				struct hw_buffer_descriptor *next = d->next;
				clear_rx_descriptor(d);
				__sync_synchronize();
				last->next = d;
				last = d;
				d = next;
			}

			g_rx_last = last;
			g_rx_next = d;
			HW_CPSW_CPDMA.CPDMA_EOI_VECTOR = end;
		}
		break;
	}
}

void setup() {
	enable_interconnects();
	enable_uart4();
	init_printf(NULL, &debug_putc);
	enable_gpio3();
	set_switch_to_ground();
	enable_dmtimer_1ms();
	enable_eth();
	wait_for_link();
}

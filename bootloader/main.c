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

static void enable_module(volatile unsigned *ctrl) {
	*ctrl |= HW_CM_MODULEMODE_ENABLE;
	while ((*ctrl & HW_CM_MODULEMODE_MASK) != HW_CM_MODULEMODE_ENABLE) {
	}
}

static void sw_wakeup(volatile unsigned *status) {
	*status |= HW_CM_CLKTRCTRL_SW_WKUP;
	while ((*status & HW_CM_CLKTRCTRL_MASK) != HW_CM_CLKTRCTRL_SW_WKUP){
	}
}

static void wait_for_functional(volatile unsigned *ctrl) {
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

static unsigned tick_count() {
	return HW_DMTIMER_1MS.TCRR;
}

static unsigned timestamp_ms() {
	// use uint64_t for increased range during the conversion
	uint64_t tick = HW_DMTIMER_1MS.TCRR;
	tick *= 1000;
	tick /= 32768;
	return (unsigned) tick;
}

static void set_sleep(unsigned target) {
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

#define ADVERTISE_OVERRIDE 0x20000000

#define ALL_NODES_0 0xFF020000
#define ALL_NODES_1 0
#define ALL_NODES_2 0
#define ALL_NODES_3 1

#define LINK_LOCAL_0 0xFE800000
#define LINK_LOCAL_1 0

#define ETH_MCAST_0 0x3333

#define ICMP6_OPTION_MAC 0x0208

struct icmp6_frame {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t body;
};

struct icmp6_advertisement {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint32_t flags;
	uint32_t addr[4];
};

struct eth_frame {
	uint16_t payloadsz; // padding for the driver, but used by the application
	uint16_t eth_dst[3];
	uint16_t eth_src[3];
	uint16_t eth_type;
	uint32_t ip6_header;
	uint16_t ip6_length;
	uint8_t ip6_next_header;
	uint8_t ip6_hop_limit;
	uint32_t ip6_src[4];
	uint32_t ip6_dst[4];
	char payload[0x5F0 - 40 - 14 - 2];
};

static_assert(sizeof(struct eth_frame) == 0x5F0, "padding");

// use almost all of the 64KB of the L3 OCMC0 memory
#define TX_FRAME_FIRST 0
#define TX_FRAME_END 16
#define RX_FRAME_FIRST 16
#define RX_FRAME_END 43

extern struct eth_frame g_eth_frames[RX_FRAME_END];
static_assert(sizeof(g_eth_frames) <= 64*1024, "too many packets");

static struct hw_buffer_descriptor *g_rx_next; // next frame to be received
static struct hw_buffer_descriptor *g_rx_last;

static struct hw_buffer_descriptor *g_tx_next; // first frame in the queue
static struct hw_buffer_descriptor *g_tx_last; // last frame we handed off to the mac
static int g_tx_free; // next frame if we want to send

// gives the next index in the circular buffer
#define NEXT_TX_FRAME(idx) (((((idx) - TX_FRAME_FIRST) + 1) % (TX_FRAME_END - TX_FRAME_FIRST)) + TX_FRAME_FIRST)

// in native byte order
static uint16_t g_my_mac[3];
static uint32_t g_my_ip[4];

static struct eth_frame *get_next_frame() {
	struct eth_frame *f = &g_eth_frames[g_tx_free];
	volatile struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[g_tx_free];
	if (d == g_tx_next) {
		return NULL; // we've run out of buffers to send
	}

	// fill out the default values
	f->eth_src[0] = g_my_mac[0];
	f->eth_src[1] = g_my_mac[1];
	f->eth_src[2] = g_my_mac[2];
	f->ip6_header = htonl(IP6_DEFAULT_HEADER);
	f->ip6_hop_limit = IP6_DEFAULT_HOP_LIMIT;
	f->ip6_src[0] = g_my_ip[0];
	f->ip6_src[1] = g_my_ip[1];
	f->ip6_src[2] = g_my_ip[2];
	f->ip6_src[3] = g_my_ip[3];
	
	return f;
}


static void send_frame(struct eth_frame *f) {
	// fill out the fixed fields
	f->eth_type = htons(ETH_IP6);
	f->ip6_length = htons(f->payloadsz);

	int idx = f - g_eth_frames;
	struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[idx];

	int framesz = f->payloadsz + offsetof(struct eth_frame, payload) - offsetof(struct eth_frame, eth_dst);
	d->next = NULL;
	d->buffer = f;
	d->offset_buf_len = ETH_OFFSET | framesz;
	d->flags_pkt_len = HW_ETH_FULL_PKT | HW_ETH_OWNED_BY_PORT | framesz;
	__sync_synchronize();

	if (g_tx_next) {
		g_tx_last->next = d;
	} else {
		// tx queue is empty
		HW_CPSW_STATERAM.TX_HDP[0] = d;
		g_tx_next = d;
	}
	
	g_tx_last = d;
	g_tx_free = NEXT_TX_FRAME(idx);
}

static void send_neighbor_advertisement() {
	struct eth_frame *f = get_next_frame();

	f->eth_dst[0] = htons(ETH_MCAST_0);
	f->eth_dst[1] = htons(hi16(ALL_NODES_3));
	f->eth_dst[2] = htons(lo16(ALL_NODES_3));
	f->ip6_dst[0] = htonl(ALL_NODES_0);
	f->ip6_dst[1] = htonl(ALL_NODES_1);
	f->ip6_dst[2] = htonl(ALL_NODES_2);
	f->ip6_dst[3] = htonl(ALL_NODES_3);
	f->ip6_next_header = IP6_ICMP;

	struct icmp6_advertisement *a = (struct icmp6_advertisement*) f->payload;
	a->type = ICMP6_NEIGHBOR_ADVERTISEMENT;
	a->code = 0;
	a->checksum = 0;
	a->flags = htonl(ADVERTISE_OVERRIDE);
	a->addr[0] = g_my_ip[0];
	a->addr[1] = g_my_ip[1];
	a->addr[2] = g_my_ip[2];
	a->addr[3] = g_my_ip[3];

	uint16_t *u = (uint16_t*) (a + 1);
	u[0] = htons(ICMP6_OPTION_MAC);
	u[1] = g_my_mac[0];
	u[2] = g_my_mac[1];
	u[3] = g_my_mac[2];

	f->payloadsz = 8 + sizeof(*a);

	printf("%u send\n", timestamp_ms());
	send_frame(f);
}

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
	
	HW_CPSW_CPDMA->CPDMA_SOFT_RESET = 1;
	while (HW_CPSW_CPDMA->CPDMA_SOFT_RESET) {}
	
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

	g_my_mac[0] = lo16(HW_CONTROL.mac[0].hi);
	g_my_mac[1] = hi16(HW_CONTROL.mac[0].hi);
	g_my_mac[2] = HW_CONTROL.mac[0].lo;

	// generate our link-local IP from the mac address
	// these are in network order
	uint32_t iid_hi = 0xFF000000 | (HW_CONTROL.mac[0].hi & 0xFFFFFF);
	uint32_t iid_lo = (HW_CONTROL.mac[0].lo << 16) | ((HW_CONTROL.mac[0].hi >> 16) & 0xFF00) | 0xFE;
	// complement the U/L bit position
	iid_hi = (iid_hi & ~2U) | (~iid_hi & 2U);

	g_my_ip[0] = htonl(LINK_LOCAL_0);
	g_my_ip[1] = htonl(LINK_LOCAL_1);
	g_my_ip[2] = iid_hi;
	g_my_ip[3] = iid_lo;

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
		struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[i];
		struct eth_frame *f = &g_eth_frames[i];
		d->buffer = f;
		clear_rx_descriptor(d);
		
		if (i > RX_FRAME_FIRST) {
			HW_BUFFER_DESCRIPTORS[i-1].next = d;
		}
	}
	g_rx_next = &HW_BUFFER_DESCRIPTORS[RX_FRAME_FIRST];
	g_rx_last = &HW_BUFFER_DESCRIPTORS[RX_FRAME_END-1];

	g_tx_next = NULL;
	g_tx_free = TX_FRAME_FIRST;

	HW_CPSW_CPDMA->RX_BUFFER_OFFSET = offsetof(struct eth_frame, eth_dst);
	HW_CPSW_STATERAM.RX_HDP[0] = &HW_BUFFER_DESCRIPTORS[RX_FRAME_FIRST];

	// enable rx interrupt
	HW_CPSW_WR.PORT_INT_ENABLE[0].RX = 1;
	HW_CPSW_CPDMA->RX_INTMASK_SET = HW_CPDMA_RX_PEND_0;
	enable_interrupt(HW_INT_ETH_RX);

	// enable tx interrupt
	HW_CPSW_WR.PORT_INT_ENABLE[0].TX = 1;
	HW_CPSW_CPDMA->TX_INTMASK_SET = HW_CPDMA_TX_PEND_0;
	enable_interrupt(HW_INT_ETH_TX);

	// enable the port
	HW_CPSW_CPDMA->TX_CONTROL = HW_CPDMA_TX_EN;
	HW_CPSW_CPDMA->RX_CONTROL = HW_CPDMA_RX_EN;
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
	// sz does not include the padding bytes at the beginning
	sz -= offsetof(struct eth_frame, payload) - offsetof(struct eth_frame, eth_dst);
	if (sz < 0 || ntohs(f->eth_type) != ETH_IP6)  {
		return;
	}

	printf("%u have frame from %08x%08x%08x%08x to %08x%08x%08x%08x\n", 
		timestamp_ms(),
		ntohl(f->ip6_src[0]),
		ntohl(f->ip6_src[1]),
		ntohl(f->ip6_src[2]),
		ntohl(f->ip6_src[3]),
		ntohl(f->ip6_dst[0]),
		ntohl(f->ip6_dst[1]),
		ntohl(f->ip6_dst[2]),
		ntohl(f->ip6_dst[3]));
}

void interrupt() {
	switch (HW_INTC.SIR_IRQ & HW_INTC_ACTIVE_IRQ_MASK) {
	case HW_INT_TINT1_1MS: {
			int sts = read_mdio(0, MDIO_BASIC_STATUS);
			printf("%u timer %x\n", timestamp_ms(), sts);
			send_neighbor_advertisement();
			// reset the timer interrupt
			HW_DMTIMER_1MS.TISR |= HW_1MS_INT_COMPARE;
			set_sleep(tick_count() + MS_TO_TICKS(1000));
		}
		break;
	case HW_INT_ETH_TX: {
			printf("%u send complete\n", timestamp_ms());
			// update g_tx_next to wherever the mac got up to
			struct hw_buffer_descriptor *d = g_tx_next;
			struct hw_buffer_descriptor *last_processed = d;
			while (d) {
				unsigned flags = d->flags_pkt_len;
				if (flags & HW_ETH_OWNED_BY_PORT) {
					// port hasn't sent this yet
					break;
				}
				last_processed = d;
				d = d->next;
				if (d && (flags & HW_ETH_END_OF_QUEUE)) {
					// port ran out before we added the next one
					// add it back in and keep on processing
					HW_CPSW_STATERAM.TX_HDP[0] = d;
				}
			}

			g_tx_next = d;
			HW_CPSW_STATERAM.TX_CP[0] = last_processed;
			HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_TX;
		}
		break;
	case HW_INT_ETH_RX: {
			// process each frame one by one, readding as we go
			struct hw_buffer_descriptor *d = g_rx_next;
			struct hw_buffer_descriptor *last = g_rx_last;

			for (;;) {
				unsigned flags = d->flags_pkt_len;
				if (flags & HW_ETH_OWNED_BY_PORT) {
					// we've processed all that are ready
					break;
				}
				if (flags & HW_ETH_END_OF_QUEUE) {
					// receiver ran out of packets, reinitiate the queue
					HW_CPSW_STATERAM.RX_HDP[0] = d->next;
				}

				if ((flags & (HW_ETH_FULL_PKT | HW_ETH_ERROR_MASK)) == (HW_ETH_FULL_PKT | HW_ETH_NO_ERROR)) {
					// we shouldn't be getting partial frames
					// in case we do, drop them
					int sz = d->offset_buf_len & 0xFFFF;
					if (flags & HW_ETH_PASS_CRC) {
						sz -= 4;
					}
					process_frame((struct eth_frame*) d->buffer, sz);
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
			HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_RX;
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
	printf("setup done\n");
}

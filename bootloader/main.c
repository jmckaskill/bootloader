#include "am335x.h"
#include "../printf/tinyprintf.h"
#include "../mdns/mdns.h"
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include "endian.h"

extern void interrupt();
extern void setup();

void led_wobble(void) {
	HW_GPIO_1->SETDATAOUT = 1 << 23;

    for (;;) {
        for (int i = 0; i < 1000000; i++) {

        }
		HW_GPIO_1->DATAOUT = ~HW_GPIO_1->DATAOUT;
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

static void enable_uart4() {
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
	//HW_UART_4_OP->MCR |= HW_UART_MCR_LOOPBACK_EN;
	HW_UART_4->MDR1 = HW_UART_MDR1_MODE_UART16X;

	// at this point we are in operational mode
}
#endif

static void enable_gpio3() {
	enable_module(&HW_CM_PER->GPIO3);
	wait_for_functional(&HW_CM_PER->GPIO3);
}

static void set_switch_to_ground() {
	HW_CONTROL->conf_mcasp0_fsr = HW_CONTROL_MUXMODE_7 | HW_CONTROL_DISABLE_PULL;
	HW_GPIO_3->INPUT_EN &= ~(1 << 19);
	HW_GPIO_3->SETDATAOUT |= (1 << 19);
}

#define MS_TO_TICKS(ms) ((ms)*32768/1000)

static unsigned tick_count() {
	return HW_DMTIMER_1MS->TCRR;
}

static unsigned tick_ms() {
	// use uint64_t for increased range during the conversion
	uint64_t tick = HW_DMTIMER_1MS->TCRR;
	tick *= 1000;
	tick /= 32768;
	return (unsigned) tick;
}

static void set_sleep(unsigned target) {
	HW_DMTIMER_1MS->TMAR = target;
}

static inline void enable_interrupt(enum hw_interrupt idx) {
	HW_INTC->BANK[idx >> 5].MIR_CLEAR |= 1 << ((unsigned)idx & 0x1F);
}

static inline void disable_interrupt(enum hw_interrupt idx) {
	HW_INTC->BANK[idx >> 5].MIR_SET |= 1 << ((unsigned)idx & 0x1F);
}

static void enable_dmtimer_1ms() {
	enable_module(&HW_CM_WAKEUP->TIMER1_CLKCTRL);
	wait_for_functional(&HW_CM_WAKEUP->TIMER1_CLKCTRL);
	while (!(HW_CM_WAKEUP->CLKSTCTRL & HW_CM_WAKEUP_TIMER1_GCLK)) {}

	HW_DMTIMER_1MS->TIOCP_CFG |= HW_1MS_CFG_SOFT_RESET;
	while (!(HW_DMTIMER_1MS->TISTAT & HW_1MS_TISTAT_RESET_DONE)) {}

	// Setup the clock to use the 32k external clock with no division
	HW_CM_DPLL->CLKSEL_TIMER1MS_CLK = HW_CLKSEL_TIMER1MS_32K_EXT;

	// enable the interrupt
	HW_DMTIMER_1MS->TIER |= HW_1MS_INT_COMPARE;
	enable_interrupt(HW_INT_TINT1_1MS);

	set_sleep(tick_count() + MS_TO_TICKS(1000));

	// enable the timer in compare mode. this will trigger an interrupt whenever
	// the internal timer matches the comparison register
	HW_DMTIMER_1MS->TCLR |= HW_1MS_TCLR_COMPARE_EN | HW_1MS_TCLR_START;
}

static int read_mdio(int phy, int key) {
	HW_MDIO->USERACCESS0 = HW_MDIO_READ(phy, key);
	unsigned reg;
	do {
		reg = HW_MDIO->USERACCESS0;
	} while (reg & HW_MDIO_GO);

	if (reg & HW_MDIO_ACK) {
		return HW_MDIO_VALUE(reg);
	} else {
		return -1;
	}
}

static void write_mdio(int phy, int key, int val) {
	HW_MDIO->USERACCESS0 = HW_MDIO_WRITE(phy, key, val);
	while (!(HW_MDIO->USERACCESS0 & HW_MDIO_GO)) {}
}

#define ETH_OFFSET (2 << 16)
#define ETH_IP6 0x86DD
#define IP6_TCP 6
#define IP6_UDP 17
#define IP6_ICMP 58
#define IP6_DEFAULT_HEADER 0x60000000
#define IP6_DEFAULT_HOP_LIMIT 255


struct udp_frame {
	uint8_t src_port[2];
	uint8_t dst_port[2];
	uint8_t length[2];
	uint8_t checksum[2];
	uint8_t data[1];
};

#define UDP_FRAME_SIZE offsetof(struct udp_frame, data)

struct tcp_frame {
	uint8_t src_port[2];
	uint8_t dst_port[2];
	uint8_t seq_num[4];
	uint8_t ack_num[4];
	uint8_t flags[2];
	uint8_t window[2];
	uint8_t checksum[2];
	uint8_t urgent[2];
	uint8_t data[1];
};

typedef struct {
	uint8_t u[16];
} ip6_addr_t;

typedef struct {
	uint8_t u[6];
} mac_addr_t;

#define ICMP6_NEIGHBOR_SOLICITATION 0x8700
#define ICMP6_NEIGHBOR_ADVERTISEMENT 0x8800
#define ICMP6_ECHO_REQUEST 0x8000
#define ICMP6_ECHO_REPLY 0x8100

#define ADVERTISE_OVERRIDE 0x20000000U

static const ip6_addr_t g_all_nodes = {{0xFF,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
static const ip6_addr_t g_mdns      = {{0xFF,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0xFB}};
static const mac_addr_t g_all_nodes_mac = {{0x33,0x33,0,0,0,1}};
static const mac_addr_t g_mdns_mac      = {{0x33,0x33,0,0,0,0xFB}};

#define MDNS_PORT 5353

#define LINK_LOCAL_SUBNET UINT64_C(0xFE80000000000000)

#define ETH_MCAST      UINT64_C(0x333300000000)
#define ETH_MCAST_MASK UINT64_C(0x0000FFFFFFFF)
#define ETH_UL_BIT     UINT64_C(0x020000000000)

#define ICMP6_OPTION_MAC 2

struct icmp6_frame {
	uint8_t type_code[2];
	uint8_t checksum[2];
	uint8_t body[4];
};

struct icmp6_neighbor {
	uint8_t type_code[2];
	uint8_t checksum[2];
	uint8_t flags[4];
	ip6_addr_t addr;
};

struct eth_frame {
	uint8_t padding[2];
	mac_addr_t eth_dst;
	mac_addr_t eth_src;
	uint8_t eth_type[2];
	uint8_t ip6_header[4];
	uint8_t ip6_length[2];
	uint8_t ip6_next_header;
	uint8_t ip6_hop_limit;
	ip6_addr_t ip6_src;
	ip6_addr_t ip6_dst;
	uint8_t payload[0x5F0 - 40 - 14 - 2];
};

#define ETH_FRAME_SIZE (offsetof(struct eth_frame, payload) - offsetof(struct eth_frame, eth_dst))

static_assert(sizeof(struct eth_frame) == 0x5F0, "padding");

// use almost all of the 64KB of the L3 OCMC0 memory
#define TX_FRAME_FIRST 0
#define TX_FRAME_END 16
#define RX_FRAME_FIRST 16
#define RX_FRAME_END 43

#define HW_ETH_FRAMES ((struct eth_frame*)0x40300000)
static_assert(sizeof(struct eth_frame) * RX_FRAME_END <= 64*1024, "too many packets");

static struct hw_buffer_descriptor *g_rx_next; // next frame to be received
static struct hw_buffer_descriptor *g_rx_last;

static struct hw_buffer_descriptor *g_tx_next; // first frame in the queue
static struct hw_buffer_descriptor *g_tx_last; // last frame we handed off to the mac
static int g_tx_free; // next frame if we want to send

// gives the next index in the circular buffer
#define NEXT_TX_FRAME(idx) (((((idx) - TX_FRAME_FIRST) + 1) % (TX_FRAME_END - TX_FRAME_FIRST)) + TX_FRAME_FIRST)

static void copy_mac(mac_addr_t *dst, const mac_addr_t *src) {
	// we can rely on 16 bit alignment but no more
	uint16_t *to = (uint16_t*) dst->u;
	uint16_t *from = (uint16_t*) src->u;
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
}

static void copy_ip6(ip6_addr_t *dst, const ip6_addr_t *src) {
	// ip6 addresses should be 32 bit aligned
	uint32_t *to = (uint32_t*) dst->u;
	uint32_t *from = (uint32_t*) src->u;
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
	to[3] = from[3];
}

void copy_unaligned(void *dst, const void *src, size_t sz) {
	uint8_t *to = (uint8_t*) dst;
	uint8_t *from = (uint8_t*) src;
	while (sz) {
		*(to++) = *(from++);
		sz--;
	}
}

static uint64_t g_my_mac64;
static mac_addr_t g_my_mac;
static ip6_addr_t g_my_ip;
static unsigned g_link;

static struct eth_frame *get_next_frame() {
	if (!g_link) {
		return NULL;
	}

	struct eth_frame *f = &HW_ETH_FRAMES[g_tx_free];
	volatile struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[g_tx_free];
	if (d == g_tx_next) {
		return NULL; // we've run out of buffers to send
	}

	// fill out the default values
	copy_mac(&f->eth_src, &g_my_mac);
	write_big_32(f->ip6_header, IP6_DEFAULT_HEADER);
	f->ip6_hop_limit = IP6_DEFAULT_HOP_LIMIT;
	copy_ip6(&f->ip6_src, &g_my_ip);
	
	return f;
}


static void send_frame(int payloadsz) {
	int idx = g_tx_free;
	struct eth_frame *f = &HW_ETH_FRAMES[idx];
	struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[idx];

	// fill out the fixed fields
	write_big_16(f->eth_type, ETH_IP6);
	write_big_16(f->ip6_length, payloadsz);

	int framesz = payloadsz + offsetof(struct eth_frame, payload) - offsetof(struct eth_frame, eth_dst);
	d->next = NULL;
	d->buffer = f;
	d->offset_buf_len = ETH_OFFSET | framesz;
	d->flags_pkt_len = HW_ETH_FULL_PKT | HW_ETH_OWNED_BY_PORT | framesz;
	__sync_synchronize();

	if (g_tx_next) {
		g_tx_last->next = d;
	} else {
		// tx queue is empty
		HW_CPSW_STATERAM->TX_HDP[0] = d;
		g_tx_next = d;
	}
	
	g_tx_last = d;
	g_tx_free = NEXT_TX_FRAME(idx);
	debugf("\tsend\n");
}

static uint16_t ones_checksum(struct eth_frame *f, int payloadsz) {
	uint32_t sum = 0;

	// add the psuedo headers
	uint8_t *p = f->ip6_src.u;
	for (int i = 0; i < 16; i++) {
		sum += read_big_16(p + i * 2);
	}
	sum += payloadsz;
	sum += f->ip6_next_header;

	// add the upper layer header
	p = f->payload;
	for (int i = 0; i < payloadsz / 2; i++) {
		sum += read_big_16(p + i * 2);
	}

	// zero fill the last byte if unaligned
	if (payloadsz & 1) {
		sum += p[payloadsz-1] << 8;
	}

	uint16_t sum16 = ((uint16_t) sum) + ((uint16_t) (sum >> 16));
	return ~sum16;
}

static void send_neighbor_advertisement() {
	struct eth_frame *f = get_next_frame();
	if (f) {
		copy_mac(&f->eth_dst, &g_all_nodes_mac);
		copy_ip6(&f->ip6_dst, &g_all_nodes);
		f->ip6_next_header = IP6_ICMP;

		struct icmp6_neighbor *a = (struct icmp6_neighbor*) f->payload;
		write_big_16(a->type_code, ICMP6_NEIGHBOR_ADVERTISEMENT);
		write_big_16(a->checksum, 0);
		write_big_32(a->flags, ADVERTISE_OVERRIDE);
		copy_ip6(&a->addr, &g_my_ip);
		
		uint8_t *u = (uint8_t*) (a + 1);
		u[0] = ICMP6_OPTION_MAC;
		u[1] = 1; // length including header in 8 byte units
		copy_unaligned(u+2, &g_my_mac, sizeof(g_my_mac));
		u += 8;

		int payloadsz = u - f->payload;
		write_big_16(a->checksum, ones_checksum(f, payloadsz));

		send_frame(payloadsz);
	}
}

static void send_echo_reply(struct eth_frame *req, uint32_t idseq, const void *data, int sz) {
	struct eth_frame *f = get_next_frame();
	if (f) {
		copy_mac(&f->eth_dst, &req->eth_src);
		copy_ip6(&f->ip6_dst, &req->ip6_src);
		f->ip6_next_header = IP6_ICMP;

		struct icmp6_frame *a = (struct icmp6_frame*) f->payload;
		write_big_16(a->type_code, ICMP6_ECHO_REPLY);
		write_big_16(a->checksum, 0);
		write_big_32(a->body, idseq);
		copy_unaligned(a+1, data, sz);

		int payloadsz = sizeof(*a) + sz;
		write_big_16(a->checksum, ones_checksum(f, payloadsz));

		send_frame(payloadsz);
	}
}

static void clear_rx_descriptor(struct hw_buffer_descriptor *d) {
	d->next = NULL;
	d->offset_buf_len = HW_ETH_MAX_LEN;
	d->flags_pkt_len = HW_ETH_OWNED_BY_PORT;
}

static uint64_t swap_48(unsigned hi32, unsigned lo16) {
	uint8_t v[6];
	write_little_32(v, hi32);
	write_little_16(v+4, lo16);
	return read_big_48(v);
}

static void reset_rx_queue() {
	for (int i = RX_FRAME_FIRST; i < RX_FRAME_END; i++) {
		struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[i];
		struct eth_frame *f = &HW_ETH_FRAMES[i];
		d->buffer = f;
		clear_rx_descriptor(d);
		
		if (i > RX_FRAME_FIRST) {
			HW_BUFFER_DESCRIPTORS[i-1].next = d;
		}
	}
	g_rx_next = &HW_BUFFER_DESCRIPTORS[RX_FRAME_FIRST];
	g_rx_last = &HW_BUFFER_DESCRIPTORS[RX_FRAME_END-1];
	
	HW_CPSW_STATERAM->RX_HDP[0] = &HW_BUFFER_DESCRIPTORS[RX_FRAME_FIRST];
}

static const char hex[] = "0123456789abcdef";

static unsigned g_last_mdns;
static struct emdns_service g_mdns_services[1] = {
	EMDNS_SERVICE("\x05_http\x04_tcp\x05local\0", "\0", 80),
};
#define PFX "ethboot_"
static char g_mdns_label[] = PFX "001122334455";
static char g_mdns_host[] = "\0" PFX "001122334455\x05local\0";
static struct emdns_responder g_mdns_responder = {
	NULL, 0,
	(emdns_ip6_t*) &g_my_ip, 1,
	g_mdns_services, 1,
	g_mdns_label, sizeof(g_mdns_label)-1,
	(uint8_t*) g_mdns_host, sizeof(g_mdns_host)-1,
};

static void send_mdns_broadcast() {
	struct eth_frame *f = get_next_frame();
	if (f) {
		copy_mac(&f->eth_dst, &g_mdns_mac);
		copy_ip6(&f->ip6_dst, &g_mdns);
		f->ip6_next_header = IP6_UDP;

		struct udp_frame *udp = (struct udp_frame*) f->payload;
		write_big_16(udp->src_port, MDNS_PORT);
		write_big_16(udp->dst_port, MDNS_PORT);
		int mdnssz = emdns_build_response(&g_mdns_responder, udp->data, HW_ETH_MAX_LEN - ETH_FRAME_SIZE - UDP_FRAME_SIZE);
		int udpsz = mdnssz + UDP_FRAME_SIZE;
		write_big_16(udp->length, udpsz);
		write_big_16(udp->checksum, 0);
		write_big_16(udp->checksum, ones_checksum(f, udpsz));

		send_frame(udpsz);
		g_last_mdns = tick_count();
	}
}

static void init_mdns() {
	char *label = g_mdns_label + sizeof(PFX) - 1;
	char *host = g_mdns_host + 1 + sizeof(PFX) - 1;
	for (int i = 0; i < 6; i++) {
		host[2*i] = label[2*i] = hex[g_my_mac.u[i] >> 4];
		host[2*i+1] = label[2*i+1] = hex[g_my_mac.u[i] & 15];
	}
	g_mdns_host[0] = sizeof(PFX)-1 + 12;

	send_mdns_broadcast();
}

static int g_announce_left;

static void check_link() {
	unsigned link = HW_MDIO->LINK;
	debugf("\tcheck link %x vs %x\n", link, g_link);

	if (link == g_link) {
		// no change
	} else if (link) {
		debugf("\tmdio link up\n");
		enable_interrupt(HW_INT_ETH_RX);
		enable_interrupt(HW_INT_ETH_TX);
		g_link = link;

		send_neighbor_advertisement();
		init_mdns();
		g_announce_left = 2;
	} else {
		debugf("\tmdio link down\n");
		disable_interrupt(HW_INT_ETH_RX);
		disable_interrupt(HW_INT_ETH_TX);

		// clear the tx channel
		struct hw_buffer_descriptor *d = g_tx_next;
		if (d) {
			HW_CPSW_CPDMA->TX_TEARDOWN = HW_CPDMA_TEARDOWN_0;
			// wait for the mac to acknowledge the teardown
			while (HW_CPSW_STATERAM->TX_CP[0] != HW_CPDMA_TEARDOWN_COMPLETE) {}
			// write the teardown back to TX_CP to stop the interrupt
			HW_CPSW_STATERAM->TX_CP[0] = HW_CPDMA_TEARDOWN_COMPLETE;
			g_tx_next = NULL;
			g_tx_last = NULL;
			g_tx_free = TX_FRAME_FIRST;
		}

		// reset the rx channel
		HW_CPSW_CPDMA->RX_TEARDOWN = HW_CPDMA_TEARDOWN_0;
		while (HW_CPSW_STATERAM->RX_CP[0] != HW_CPDMA_TEARDOWN_COMPLETE) {}
		HW_CPSW_STATERAM->RX_CP[0] = HW_CPDMA_TEARDOWN_COMPLETE;
		reset_rx_queue();

		g_link = link;
	}
}

static void enable_eth() {
	HW_CONTROL->gmii_sel = HW_CONTROL_GMII_1 | HW_CONTROL_GMII_2;

	enable_module(&HW_CM_PER->CPGMAC0);
	sw_wakeup(&HW_CM_PER->CPSW_status);
	wait_for_functional(&HW_CM_PER->CPGMAC0);
	while (!(HW_CM_PER->CPSW_status & HW_CM_CPSW_125MHZ_GCLK)) {}
	while (!(HW_CM_PER->L4HS_status & HW_CM_L4HS_CPSW_5MHZ_GCLK)) {}
	while (!(HW_CM_PER->L4HS_status & HW_CM_L4HS_CPSW_50MHZ_GCLK)) {}
	while (!(HW_CM_PER->L4HS_status & HW_CM_L4HS_CPSW_25MHZ_GCLK)) {}

	// reset the various ethernet subsystems
	// resetting the wrapper also resets the MDIO module
	HW_CPSW_WR->SOFT_RESET = 1;
	HW_CPSW_WR->CONTROL = HW_WR_CONTROL_NO_STANDBY | HW_WR_CONTROL_NO_IDLE;
	
	HW_CPSW_CPDMA->CPDMA_SOFT_RESET = 1;
	while (HW_CPSW_CPDMA->CPDMA_SOFT_RESET) {}
	
	HW_CPSW_SL_1->SOFT_RESET = 1;
	HW_CPSW_SL_2->SOFT_RESET = 1;
	while (HW_CPSW_SL_1->SOFT_RESET) {}
	while (HW_CPSW_SL_2->SOFT_RESET) {}

	HW_CPSW_SS->SOFT_RESET = 1;
	while (HW_CPSW_SS->SOFT_RESET) {}

	// start setting stuff back up
	HW_CPSW_ALE->CONTROL = HW_ALE_CONTROL_CLEAR | HW_ALE_CONTROL_EN;
	HW_CPSW_ALE->PORTCTL[0] = HW_ALE_PORT_FORWARD;
	HW_CPSW_ALE->PORTCTL[1] = HW_ALE_PORT_FORWARD;

	HW_CPSW_PORT_1->SA_LO = HW_CONTROL->mac[0].lo;
	HW_CPSW_PORT_1->SA_HI = HW_CONTROL->mac[0].hi;

	// first byte on the wire is in hi bits 0-7
	// so need to swap to native byte order
	g_my_mac64 = swap_48(HW_CONTROL->mac[0].hi, HW_CONTROL->mac[0].lo);
	write_big_48(g_my_mac.u, g_my_mac64);

	// now generate the interface ID from the mac
	// complement the U/L bit position
	uint64_t mac_ul = g_my_mac64 ^ ETH_UL_BIT;
	// now split the bottom and top haves and put 0xFFFE in the middle
	uint64_t iid = ((mac_ul & UINT64_C(0xFFFFFF000000)) << 16) 
				 | UINT64_C(0xFFFE000000)
				 | ((mac_ul & UINT64_C(0xFFFFFF)));

	uint128_t ip;
	ip.low = iid;
	ip.high = LINK_LOCAL_SUBNET;
	write_big_128(g_my_ip.u, ip);

	// main CPSW clock is 125 MHz. this gives an MDIO frequency of 2.5 MHz
	// which matches what the ROM does and the max frequency of the PHY
	// this also puts the other fields back to default values
	HW_MDIO->CONTROL = (HW_MDIO->CONTROL &~ HW_MDIO_CLKDIV_MASK) | HW_MDIO_CLKDIV(49);
	HW_MDIO->CONTROL |= HW_MDIO_CONTROL_EN;
	HW_MDIO->USERPHYSEL0 = HW_MDIO_USERPHYSEL_ADDR(0) | HW_MDIO_LINK_INT_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].MISC = HW_WR_MISC_INT_MDIO_LINK;
	enable_interrupt(HW_INT_ETH_MISC);
	
	// setup the phy
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_SOFT_RESET);
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_AUTO_NEGOTIATION | MDIO_CTL_RESTART_AUTO);

	// setup rx
	HW_CPSW_CPDMA->RX_CONTROL = HW_CPDMA_RX_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].RX = 1;
	HW_CPSW_CPDMA->RX_INTMASK_SET = HW_CPDMA_RX_PEND_0;
	HW_CPSW_CPDMA->RX_BUFFER_OFFSET = offsetof(struct eth_frame, eth_dst);
	reset_rx_queue();
	
	// setup tx
	g_tx_next = NULL;
	g_tx_last = NULL;
	g_tx_free = TX_FRAME_FIRST;
	HW_CPSW_CPDMA->TX_CONTROL = HW_CPDMA_TX_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].TX = 1;
	HW_CPSW_CPDMA->TX_INTMASK_SET = HW_CPDMA_TX_PEND_0;
	
	// enable the sliver
	HW_CPSW_SL_1->MACCONTROL = HW_SL_MACCONTROL_MII_EN | HW_SL_MACCONTROL_FULL_DUPLEX;
	while (!(HW_CPSW_SL_1->MACCONTROL & HW_SL_MACCONTROL_MII_EN)) {}
	
	// we don't enable the interrupts until the link comes online
	// that way we always process the link change before any rx messages
	// that come really early
	g_link = 0;
}

#ifdef DEBUG_LOG_ENABLED
static void debug_putc(void* udata, char ch) {
	while ((HW_UART_4_OP->LSR & HW_UART_LSR_TX_EMPTY) != HW_UART_LSR_TX_EMPTY) {}
	HW_UART_4_OP->RHR_THR = ch;
}
#endif


#define ADDR_LEN (8*5)
static char *print_addr(char *buf, const ip6_addr_t *v) {
	char *p = buf;
	int section = -1; // -1,0,1 for before,in,after the compressed zero section
	for (int i = 0; i < 16; i += 2) {
		uint16_t u = read_big_16(v->u+i);
		if (!u) {
			if (section == 0) {
				// we are in an already started zeroes section
			} else if (section < 0) {
				// we can start the zeroes section
				if (i == 0) {
					*(p++) = ':';
				}
				*(p++) = ':';
				section = 0;
			} else {
				// we've already completed the zeroes section
				*(p++) = '0';
				*(p++) = ':';
			}
		} else {
			if (section == 0) {
				section = 1;
			}
			if (u < 0x10) {
				goto print_1;
			} else if (u < 0x100) {
				goto print_2;
			} else if (u < 0x1000) {
				goto print_3;
			}
			*(p++) = hex[u >> 12];
		print_3:
			*(p++) = hex[(u >> 8) & 15];
		print_2:
			*(p++) = hex[(u >> 4) & 15];
		print_1:
			*(p++) = hex[u & 15];
			*(p++) = ':';
		}
	}
	p[section ? -1 : 0] = '\0';
	return buf;
}

#define MAC_LEN (6*3)
static char *print_mac(char *buf, const mac_addr_t *v) {
	for (int i = 0; i < 6; i++) {
		*(buf++) = hex[v->u[i] >> 4];
		*(buf++) = hex[v->u[i] & 15];
		*(buf++) = ':';
	}
	buf[-1] = '\0';
	return buf - MAC_LEN;
}

static void process_frame(struct eth_frame *f, int sz) {
	// sz does not include the padding bytes at the beginning
	sz -= offsetof(struct eth_frame, payload) - offsetof(struct eth_frame, eth_dst);
	if (sz < 0 || read_big_16(f->eth_type) != ETH_IP6)  {
		return;
	}

	char ip6_src[ADDR_LEN], ip6_dst[ADDR_LEN];
	char mac_src[MAC_LEN], mac_dst[MAC_LEN];

	debugf("\thave frame from %s/%s to %s/%s size %d\n", 
		print_mac(mac_src, &f->eth_src),
		print_addr(ip6_src, &f->ip6_src),
		print_mac(mac_dst, &f->eth_dst),
		print_addr(ip6_dst, &f->ip6_dst),
		sz);

	switch (f->ip6_next_header) {
	case IP6_UDP:
		if (sz >= sizeof(struct udp_frame)) {
			struct udp_frame *udp = (struct udp_frame*) f->payload;
			int msgsz = sz - UDP_FRAME_SIZE;
			int src_port = read_big_16(udp->src_port);
			int dst_port = read_big_16(udp->dst_port);

			debugf("\thave udp from %d to %d\n", src_port, dst_port);

			if (src_port == MDNS_PORT
			 && dst_port == MDNS_PORT
			 && !memcmp(&f->ip6_dst, &g_mdns, sizeof(f->ip6_dst))
			 && (tick_count() - g_last_mdns > MS_TO_TICKS(1000))
			 && emdns_should_respond(&g_mdns_responder, udp->data, msgsz) == EMDNS_RESPOND) {
				 send_mdns_broadcast();
			}

		}
		break;
	case IP6_ICMP:
		if (sz >= sizeof(struct icmp6_frame)) {
			struct icmp6_frame *icmp = (struct icmp6_frame*) f->payload;
			unsigned type = read_big_16(icmp->type_code);
			debugf("\thave icmp type %x\n", type);
			switch (type) {
			case ICMP6_ECHO_REQUEST:
				send_echo_reply(f, read_big_32(icmp->body), icmp+1, sz - sizeof(*icmp));
				break;
			case ICMP6_NEIGHBOR_SOLICITATION:
				if (sz >= sizeof(struct icmp6_neighbor)) {
					struct icmp6_neighbor *n = (struct icmp6_neighbor*) f->payload;
					if (!memcmp(&n->addr, &g_my_ip, sizeof(n->addr))) {
						send_neighbor_advertisement();
					}
				}
				break;
			}
		}
		break;
	}
}

void interrupt() {
	unsigned hwint = HW_INTC->SIR_IRQ & HW_INTC_ACTIVE_IRQ_MASK;
	debugf("%u interrupt %u\n", tick_ms(), hwint);
	switch (hwint) {
	case HW_INT_TINT1_1MS: {
			debugf("\ttimer\n");
			if (g_link && g_announce_left) {
				send_neighbor_advertisement();
				send_mdns_broadcast();
				g_announce_left--;
			}
			// reset the timer interrupt
			HW_DMTIMER_1MS->TISR |= HW_1MS_INT_COMPARE;
			set_sleep(tick_count() + MS_TO_TICKS(1000));
		}
		break;
	case HW_INT_ETH_TX: {
			debugf("\tsend complete\n");
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
					HW_CPSW_STATERAM->TX_HDP[0] = d;
				}
			}

			g_tx_next = d;
			HW_CPSW_STATERAM->TX_CP[0] = last_processed;
			HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_TX;
		}
		break;
	case HW_INT_ETH_RX: {
			// process each frame one by one, readding as we go
			struct hw_buffer_descriptor *d = g_rx_next;
			struct hw_buffer_descriptor *last = g_rx_last;

			for (;;) {
				unsigned flags = d->flags_pkt_len;
				debugf("\thave rx packet flags %x\n", flags);
				if (flags & HW_ETH_OWNED_BY_PORT) {
					// we've processed all that are ready
					break;
				}
				if (flags & HW_ETH_END_OF_QUEUE) {
					// receiver ran out of packets, reinitiate the queue
					HW_CPSW_STATERAM->RX_HDP[0] = d->next;
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
			HW_CPSW_STATERAM->RX_CP[0] = last;
			HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_RX;
		}
		break;
	case HW_INT_ETH_MISC: {
			if (HW_CPSW_WR->PORT_INT_STATUS[0].MISC == HW_WR_MISC_INT_MDIO_LINK) {
				check_link();
				HW_MDIO->LINKINTRAW = 1;
			}
			HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_MISC;
		}
		break;
	}
}

void setup() {
	enable_interconnects();

#ifdef DEBUG_LOG_ENABLED
	enable_uart4();
	init_printf(NULL, &debug_putc);
#endif

	//enable_gpio3();
	//set_switch_to_ground();
	enable_dmtimer_1ms();
	enable_eth();
	check_link();
	debugf("setup done\n");
}

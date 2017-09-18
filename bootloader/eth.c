#include "eth.h"
#include "board.h"
#include "debug.h"
#include "../mdns/endian.h"

mac_addr_t g_my_mac;
ip6_addr_t g_my_ip;
static unsigned g_link_status;

// use almost all of the 64KB of the L3 OCMC0 memory
#define TX_BUFFER_FIRST 0
#define TX_BUFFER_END 16
#define RX_BUFFER_FIRST 16
#define RX_BUFFER_END 43

struct eth_buffer {
	uint8_t padding[2];
	struct ip6_header hdr;
	uint8_t payload[0x5F0 - sizeof(struct ip6_header) - 2];
};

#define HW_ETH_BUFFERS ((struct eth_buffer*)0x40300000)
static_assert(sizeof(struct eth_buffer) == 0x5F0, "padding");
static_assert(sizeof(struct eth_buffer) * RX_BUFFER_END <= 64*1024, "too many packets");

static struct hw_buffer_descriptor *g_rx_next; // next frame to be received
static struct hw_buffer_descriptor *g_rx_last;

static struct hw_buffer_descriptor *g_tx_next; // first frame in the queue
static struct hw_buffer_descriptor *g_tx_last; // last frame we handed off to the mac
static int g_tx_free; // next frame if we want to send

// gives the next index in the circular buffer
#define NEXT_TX_BUFFER(idx) (((((idx) - TX_BUFFER_FIRST) + 1) % (TX_BUFFER_END - TX_BUFFER_FIRST)) + TX_BUFFER_FIRST)

struct ip6_header *new_eth_frame(const mac_addr_t *dst_mac, const ip6_addr_t *dst_ip, uint8_t type) {
	if (!g_link_status) {
		return NULL;
	}

	struct eth_buffer *b = &HW_ETH_BUFFERS[g_tx_free];
	volatile struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[g_tx_free];
	if (d == g_tx_next) {
		return NULL; // we've run out of buffers to send
	}

	// fill out the default values
	struct ip6_header *e = &b->hdr;
	copy_mac(&e->eth_dst, dst_mac);
	copy_mac(&e->eth_src, &g_my_mac);
	write_big_32(e->ip6_header, IP6_DEFAULT_HEADER);
	e->ip6_hop_limit = IP6_DEFAULT_HOP_LIMIT;
	e->ip6_next_header = type;
	copy_ip6(&e->ip6_dst, dst_ip);
	copy_ip6(&e->ip6_src, &g_my_ip);
	
	return e;
}

void send_eth_frame(int sz) {
	struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[g_tx_free];
	struct eth_buffer *b = &HW_ETH_BUFFERS[g_tx_free];

	// fill out the fixed fields
	write_big_16(b->hdr.eth_type, ETH_IP6);
	write_big_16(b->hdr.ip6_length, sz);

	int framesz = sz + sizeof(b->hdr);
	d->next = NULL;
	d->buffer = b;
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
	g_tx_free = NEXT_TX_BUFFER(g_tx_free);
	debugf("\tsend\n");
}


static void clear_rx_descriptor(struct hw_buffer_descriptor *d) {
	d->next = NULL;
	d->offset_buf_len = HW_ETH_MAX_LEN;
	d->flags_pkt_len = HW_ETH_OWNED_BY_PORT;
}

static void reset_rx_queue() {
	for (int i = RX_BUFFER_FIRST; i < RX_BUFFER_END; i++) {
		struct hw_buffer_descriptor *d = &HW_BUFFER_DESCRIPTORS[i];
		struct eth_buffer *f = &HW_ETH_BUFFERS[i];
		d->buffer = f;
		clear_rx_descriptor(d);
		
		if (i > RX_BUFFER_FIRST) {
			HW_BUFFER_DESCRIPTORS[i-1].next = d;
		}
	}
	g_rx_next = &HW_BUFFER_DESCRIPTORS[RX_BUFFER_FIRST];
	g_rx_last = &HW_BUFFER_DESCRIPTORS[RX_BUFFER_END-1];
	
	HW_CPSW_STATERAM->RX_HDP[0] = &HW_BUFFER_DESCRIPTORS[RX_BUFFER_FIRST];
}

void check_ethernet_link() {
	unsigned link = HW_MDIO->LINK;
	debugf("\tcheck link %x previous %x\n", link, g_link_status);

	if (link == g_link_status) {
		// no change
	} else if (link) {
		debugf("\tmdio link up\n");
		hw_enable_interrupt(HW_INT_ETH_RX);
		hw_enable_interrupt(HW_INT_ETH_TX);
		g_link_status = link;

        ethernet_up();
	} else {
		debugf("\tmdio link down\n");
		hw_disable_interrupt(HW_INT_ETH_RX);
		hw_disable_interrupt(HW_INT_ETH_TX);

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
			g_tx_free = TX_BUFFER_FIRST;
		}

		// reset the rx channel
		HW_CPSW_CPDMA->RX_TEARDOWN = HW_CPDMA_TEARDOWN_0;
		while (HW_CPSW_STATERAM->RX_CP[0] != HW_CPDMA_TEARDOWN_COMPLETE) {}
		HW_CPSW_STATERAM->RX_CP[0] = HW_CPDMA_TEARDOWN_COMPLETE;
		reset_rx_queue();

		g_link_status = link;

        ethernet_down();
	}
}

void process_eth_misc_interrupt() {
	if (HW_CPSW_WR->PORT_INT_STATUS[0].MISC == HW_WR_MISC_INT_MDIO_LINK) {
		check_ethernet_link();
		HW_MDIO->LINKINTRAW = 1;
	}
	HW_CPSW_CPDMA->CPDMA_EOI_VECTOR = HW_CPDMA_EOI_MISC;
}

static void write_mdio(int phy, int key, int val) {
	HW_MDIO->USERACCESS0 = HW_MDIO_WRITE(phy, key, val);
	while (!(HW_MDIO->USERACCESS0 & HW_MDIO_GO)) {}
}

void init_ethernet() {
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
    write_little_32(g_my_mac.u, HW_CONTROL->mac[0].hi);
    write_little_16(g_my_mac.u+4, HW_CONTROL->mac[0].lo);

	// now generate the interface ID from the mac
	// complement the U/L bit position
	uint64_t mac_ul = big_48(g_my_mac.u) ^ ETH_UL_BIT;
	// now split the bottom and top halves and put 0xFFFE in the middle
	uint64_t iid = ((mac_ul & UINT64_C(0xFFFFFF000000)) << 16) 
				 | UINT64_C(0xFFFE000000)
				 | ((mac_ul & UINT64_C(0xFFFFFF)));

    write_big_64(g_my_ip.u, LINK_LOCAL_SUBNET);
    write_big_64(g_my_ip.u+8, iid);

	// main CPSW clock is 125 MHz. this gives an MDIO frequency of 2.5 MHz
	// which matches what the ROM does and the max frequency of the PHY
	// this also puts the other fields back to default values
	HW_MDIO->CONTROL = (HW_MDIO->CONTROL &~ HW_MDIO_CLKDIV_MASK) | HW_MDIO_CLKDIV(49);
	HW_MDIO->CONTROL |= HW_MDIO_CONTROL_EN;
	HW_MDIO->USERPHYSEL0 = HW_MDIO_USERPHYSEL_ADDR(0) | HW_MDIO_LINK_INT_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].MISC = HW_WR_MISC_INT_MDIO_LINK;
	hw_enable_interrupt(HW_INT_ETH_MISC);
	
	// setup the phy
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_SOFT_RESET);
	write_mdio(0, MDIO_BASIC_CONTROL, MDIO_CTL_AUTO_NEGOTIATION | MDIO_CTL_RESTART_AUTO);

	// setup rx
	HW_CPSW_CPDMA->RX_CONTROL = HW_CPDMA_RX_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].RX = 1;
	HW_CPSW_CPDMA->RX_INTMASK_SET = HW_CPDMA_RX_PEND_0;
	HW_CPSW_CPDMA->RX_BUFFER_OFFSET = offsetof(struct eth_buffer, hdr);
	reset_rx_queue();
	
	// setup tx
	g_tx_next = NULL;
	g_tx_last = NULL;
	g_tx_free = TX_BUFFER_FIRST;
	HW_CPSW_CPDMA->TX_CONTROL = HW_CPDMA_TX_EN;
	HW_CPSW_WR->PORT_INT_ENABLE[0].TX = 1;
	HW_CPSW_CPDMA->TX_INTMASK_SET = HW_CPDMA_TX_PEND_0;
	
	// enable the sliver
	HW_CPSW_SL_1->MACCONTROL = HW_SL_MACCONTROL_MII_EN | HW_SL_MACCONTROL_FULL_DUPLEX;
	while (!(HW_CPSW_SL_1->MACCONTROL & HW_SL_MACCONTROL_MII_EN)) {}
	
	// we don't enable the interrupts until the link comes online
	// that way we always process the link change before any rx messages
	// that come really early
	g_link_status = 0;
}

void process_eth_tx_interrupt() {
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

void process_eth_rx_interrupt() {
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
			struct eth_buffer *b = (struct eth_buffer*) d->buffer;
			if (sz >= sizeof(b->hdr) && big_16(b->hdr.eth_type) == ETH_IP6) {
				process_ip6(&b->hdr, b->hdr.ip6_next_header, b->payload, sz - sizeof(b->hdr));
			}
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

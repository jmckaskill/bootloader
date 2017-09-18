#include "../printf/tinyprintf.h"

#include "../mdns/copy.h"
#include "../mdns/endian.h"
#include "mdns.h"
#include "tcp.h"
#include "icmp6.h"
#include "debug.h"
#include "ip6.h"
#include "eth.h"
#include "board.h"
#include "tick.h"
#include "am335x.h"
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

extern void interrupt();
extern void setup();

static int g_announce_left;

void ethernet_up() {
	send_neighbor_advertisement();
	init_mdns();
	init_tcp();
	g_announce_left = 2;
}

void ethernet_down() {
	g_announce_left = 0;
}

#define HTTP_PORT 80

int should_accept_connection(struct ip6_header *ip, uint16_t dst_port) {
	return dst_port == HTTP_PORT;
}

static const char ALIGNED_32 hello_world[] = "HTTP/1.1 OK\r\nContent-Length:14\r\n\r\nHello World!\r\n";

int process_tcp_data(struct tcp_connection *c, const void *msg, int sz) {
	switch (c->local_port) {
	case HTTP_PORT:
		if (sz && c->tx_left) {
			// pipelines are not supported
			return -1;
		}
		if (sz) {
			c->tx_data = hello_world;
			c->tx_left = sizeof(hello_world) - 1;
		}
		return 0;
	}

	return -1;
}

void process_ip6(struct ip6_header *ip, uint8_t type, const void *msg, int sz) {
	char ip6_src[PRINT_ADDR_LEN], ip6_dst[PRINT_ADDR_LEN];
	char mac_src[PRINT_MAC_LEN], mac_dst[PRINT_MAC_LEN];
	debugf("\thave frame from %s/%s to %s/%s size %d\n", 
		print_mac(mac_src, &ip->eth_src),
		print_addr(ip6_src, &ip->ip6_src),
		print_mac(mac_dst, &ip->eth_dst),
		print_addr(ip6_dst, &ip->ip6_dst),
		sz);

	switch (type) {
	case IP6_TCP:
		process_tcp(ip, msg, sz);
		break;
	case IP6_UDP:
		if (sz >= sizeof(struct udp_header)) {
			struct udp_header *udp = (struct udp_header*) msg;
			process_mdns(ip, udp, udp+1, sz - sizeof(*udp));
		}
	case IP6_ICMP:
		process_icmp6(ip, msg, sz);
		break;
	}
}

void interrupt() {
	unsigned hwint = HW_INTC->SIR_IRQ & HW_INTC_ACTIVE_IRQ_MASK;
	debugf("%u interrupt %u\n", tick_ms(), hwint);

	switch (hwint) {
	case HW_INT_TINT1_1MS: {
			debugf("\ttimer\n");
			if (g_announce_left) {
				send_neighbor_advertisement();
				send_mdns_broadcast();
				g_announce_left--;
			}
			// reset the timer interrupt
			HW_DMTIMER_1MS->TISR |= HW_1MS_INT_COMPARE;
			set_sleep(tick_count() + MS_TO_TICKS(1000));
		}
		break;
	case HW_INT_ETH_TX:
		process_eth_tx_interrupt();
		break;
	case HW_INT_ETH_RX:
		process_eth_rx_interrupt();
		break;
	case HW_INT_ETH_MISC:
		process_eth_misc_interrupt();
		break;
	}
}

void setup() {
	init_interconnects();
	init_debug();
	init_tick();
	init_ethernet();
	check_ethernet_link();
	debugf("setup done\n");
}

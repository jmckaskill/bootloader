#include "icmp6.h"
#include "mdns.h"
#include "tcp.h"
#include "hw/board.h"
#include "hw/debug.h"
#include "hw/eth.h"
#include "hw/tick.h"
#include "hw/copy.h"
#include "hw/endian.h"


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

void process_eth_frame(void *frame, int fsz) {
	struct ip6_header *ip = (struct ip6_header*) frame;
	if (fsz < sizeof(struct ip6_header) || big_16(ip->eth_type) != ETH_IP6) {
		return;
	}

	void *msg = ip + 1;
	int sz = fsz - sizeof(*ip);
	
#ifdef DEBUG_LOG_ENABLED
	char ip6_src[PRINT_ADDR_LEN], ip6_dst[PRINT_ADDR_LEN];
	char mac_src[PRINT_MAC_LEN], mac_dst[PRINT_MAC_LEN];
#endif

	debugf("\thave frame from %s/%s to %s/%s size %d\n", 
		print_mac(mac_src, &ip->eth_src),
		print_addr(ip6_src, &ip->ip6_src),
		print_mac(mac_dst, &ip->eth_dst),
		print_addr(ip6_dst, &ip->ip6_dst),
		sz);

	switch (ip->ip6_next_header) {
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

void on_tick() {
	debugf("\ttimer\n");
	if (g_announce_left) {
		send_neighbor_advertisement();
		send_mdns_broadcast();
		g_announce_left--;
	}
}

extern void setup();

void setup() {
	init_board();
	init_debug();
	init_tick();
	init_ethernet(g_my_mac.u);

	// now generate the interface ID from the mac
	// complement the U/L bit position
	uint64_t mac_ul = big_48(g_my_mac.u) ^ ETH_UL_BIT;
	// now split the bottom and top halves and put 0xFFFE in the middle
	uint64_t iid = ((mac_ul & UINT64_C(0xFFFFFF000000)) << 16) 
				 | UINT64_C(0xFFFE000000)
				 | ((mac_ul & UINT64_C(0xFFFFFF)));

    write_big_64(g_my_ip.u, LINK_LOCAL_SUBNET);
	write_big_64(g_my_ip.u+8, iid);
	
	check_ethernet_link();
	debugf("setup done\n");
}

#include "icmp6.h"
#include "debug.h"
#include "../mdns/endian.h"
#include "../mdns/copy.h"

static const ip6_addr_t g_all_nodes = {{0xFF,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
static const mac_addr_t g_all_nodes_mac = {{0x33,0x33,0,0,0,1}};

void send_neighbor_advertisement() {
	struct ip6_header *ip = new_eth_frame(&g_all_nodes_mac, &g_all_nodes, IP6_ICMP);
	if (ip) {
		struct icmp6_neighbor *a = (struct icmp6_neighbor*) (ip+1);
		write_big_16(a->type_code, ICMP6_NEIGHBOR_ADVERTISEMENT);
		write_big_32(a->flags, ADVERTISE_OVERRIDE);
		copy_ip6(&a->addr, &g_my_ip);
		
		uint8_t *u = (uint8_t*) (a + 1);
		u[0] = ICMP6_OPTION_MAC;
		u[1] = 1; // length including header in 8 byte units
		copy_unaligned(u+2, &g_my_mac, sizeof(g_my_mac));
        
		int sz = 8 + sizeof(*a);
		write_big_16(a->checksum, 0);
		write_big_16(a->checksum, ones_checksum(ip, sz));

		send_eth_frame(sz);
	}
}

static void send_echo_reply(struct ip6_header *ip, uint32_t idseq, const void *data, int sz) {
	struct ip6_header *rep = new_eth_frame(&ip->eth_src, &ip->ip6_src, IP6_ICMP);
	if (rep) {
		struct icmp6_header *a = (struct icmp6_header*) (rep+1);
		write_big_16(a->type_code, ICMP6_ECHO_REPLY);
		write_big_32(a->body, idseq);
		copy_unaligned(a+1, data, sz);
        
		int repsz = sizeof(*a) + sz;
		write_big_16(a->checksum, 0);
		write_big_16(a->checksum, ones_checksum(rep, repsz));

		send_eth_frame(repsz);
	}
}

void process_icmp6(struct ip6_header *ip, const void *msg, int sz) {
    if (sz < sizeof(struct icmp6_header)) {
        return;
    }

    struct icmp6_header *icmp = (struct icmp6_header*) msg;
    unsigned type = big_16(icmp->type_code);
    debugf("\thave icmp type %x\n", type);
    
    switch (type) {
    case ICMP6_ECHO_REQUEST:
        send_echo_reply(ip, big_32(icmp->body), icmp+1, sz - sizeof(*icmp));
        break;
    case ICMP6_NEIGHBOR_SOLICITATION:
        if (sz >= sizeof(struct icmp6_neighbor)) {
            struct icmp6_neighbor *n = (struct icmp6_neighbor*) (icmp);
            if (ip6_equals(&n->addr, &g_my_ip)) {
                send_neighbor_advertisement();
            }
        }
        break;
    }
}

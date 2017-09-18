#include "ip6.h"
#include "hw/eth.h"
#include "hw/debug.h"
#include "hw/endian.h"

const char hex[] = "0123456789abcdef";

char *print_addr(char *buf, const ip6_addr_t *v) {
	char *p = buf;
	int section = -1; // -1,0,1 for before,in,after the compressed zero section
	for (int i = 0; i < 16; i += 2) {
		uint16_t u = big_16(v->u+i);
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

char *print_mac(char *buf, const mac_addr_t *v) {
	for (int i = 0; i < 6; i++) {
		*(buf++) = hex[v->u[i] >> 4];
		*(buf++) = hex[v->u[i] & 15];
		*(buf++) = ':';
	}
	buf[-1] = '\0';
	return buf - PRINT_MAC_LEN;
}

uint16_t ones_checksum(struct ip6_header *ip, int payloadsz) {
	uint32_t sum = 0;

	// add the psuedo headers
	uint8_t *p = ip->ip6_src.u;
	for (int i = 0; i < 16; i++) {
		sum += big_16(p + i * 2);
	}
	sum += payloadsz;
	sum += ip->ip6_next_header;

	// add the upper layer header
	p = (uint8_t*) (ip+1);
	for (int i = 0; i < payloadsz / 2; i++) {
		sum += big_16(p + i * 2);
	}

	// zero fill the last byte if unaligned
	if (payloadsz & 1) {
		sum += p[payloadsz-1] << 8;
	}

	uint16_t sum16 = ((uint16_t) sum) + ((uint16_t) (sum >> 16));
	return ~sum16;
}

mac_addr_t g_my_mac;
ip6_addr_t g_my_ip;

struct ip6_header *new_ip6_frame(const mac_addr_t *dst_mac, const ip6_addr_t *dst_ip, uint8_t type) {
	struct ip6_header *ip = (struct ip6_header*) new_eth_frame();
	if (!ip) {
		return NULL;
	}

	// fill out the default values
	copy_mac(&ip->eth_dst, dst_mac);
	copy_mac(&ip->eth_src, &g_my_mac);
	write_big_32(ip->ip6_header, IP6_DEFAULT_HEADER);
	ip->ip6_hop_limit = IP6_DEFAULT_HOP_LIMIT;
	ip->ip6_next_header = type;
	copy_ip6(&ip->ip6_dst, dst_ip);
	copy_ip6(&ip->ip6_src, &g_my_ip);

	return ip;
}
void send_ip6_frame(struct ip6_header *ip, int sz) {
	// fill out the fixed fields
	write_big_16(ip->eth_type, ETH_IP6);
	write_big_16(ip->ip6_length, sz);
	send_eth_frame(ip, sz + sizeof(*ip));
}

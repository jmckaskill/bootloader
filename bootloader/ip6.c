#include "eth.h"
#include "debug.h"

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

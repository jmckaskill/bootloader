#pragma once
#include "ip6.h"

#define IP6_ICMP 58


#define ICMP6_NEIGHBOR_SOLICITATION 0x8700
#define ICMP6_NEIGHBOR_ADVERTISEMENT 0x8800
#define ICMP6_ECHO_REQUEST 0x8000
#define ICMP6_ECHO_REPLY 0x8100

#define ADVERTISE_OVERRIDE 0x20000000U


#define ICMP6_OPTION_MAC 2

struct icmp6_header {
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

void process_icmp6(struct ip6_header *ip, const void *msg, int sz);
void send_neighbor_advertisement();

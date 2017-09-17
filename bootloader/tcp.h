#pragma once
#include "eth.h"
#include <stddef.h>

struct tcp_connection {
	struct tcp_connection *next, *prev;
	mac_addr_t remote_mac;
	ip6_addr_t remote_addr;
	uint16_t remote_port;
	uint16_t local_port;
	uint32_t remote_seq;
	uint32_t local_seq;
	size_t sent;
	size_t remote_mss;
	size_t remote_window;
	unsigned ack_timeout;
	unsigned reset_timeout;
	unsigned open : 1;
	unsigned connected : 1;
	unsigned in_finished : 1;
	unsigned out_finished : 1;
};

void init_tcp();
void process_tcp(struct ip6_header *ip, const void *msg, int sz);

extern int should_accept_connection(struct ip6_header *ip, uint16_t dst_port);

#define TCP_FIN (1 << 0)
#define TCP_SYN (1 << 1)
#define TCP_RESET (1 << 2)
#define TCP_ACK (1 << 4)
#define TCP_FLAGS_MASK (TCP_FIN|TCP_SYN|TCP_RESET|TCP_ACK)
#define TCP_OFFSET_MASK 0xC000
#define ENCODE_TCP_OFFSET(sz) (((sz)/4) << 12)
#define DECODE_TCP_OFFSET(f) ((((f) & TCP_OFFSET_MASK) >> 12) * 4)

#define TCP_OPT_NOP 1
#define TCP_OPT_MSS 2

#define TCP_DEFAULT_WINDOW 0x4000 // about 10-11 packets
#define TCP_DEFAULT_MSS 1440 // full size ethernet frame with no tcp options

struct tcp_header {
	uint8_t src_port[2];
	uint8_t dst_port[2];
	uint8_t seq_num[4];
	uint8_t ack_num[4];
	uint8_t flags[2];
	uint8_t window[2];
	uint8_t checksum[2];
	uint8_t urgent[2];
};

#define IP6_TCP 6

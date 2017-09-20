#pragma once
#include "ip6.h"
#include "hw/tick.h"
#include <stddef.h>

struct tcp_connection {
	struct tcp_connection *next, *prev;
	mac_addr_t remote_mac;
	ip6_addr_t remote_addr;
	uint16_t remote_port;
	uint16_t local_port;

	uint32_t rx_next;

	uint32_t tx_next;
	uint16_t tx_window;
	uint16_t tx_sent;
	unsigned tx_timeout;
	uint8_t tx_retries;

	// must be 32 bit aligned
	const void *tx_data;
	int tx_size;

	unsigned open : 1;
	unsigned connected : 1;

	// finishing starts with the client sending a FIN
	// we don't support server initiated FINs, only server
	// aborts
	unsigned finishing : 1;
};

extern unsigned g_tcp_timeout;

void init_tcp();
void process_tcp(struct ip6_header *ip, const void *msg, int sz);
void process_tcp_timeout(unsigned now);

extern int should_accept_connection(struct ip6_header *ip, uint16_t dst_port);
extern int process_tcp_data(struct tcp_connection *c, const void *data, int sz);

#define TCP_FIN (1 << 0)
#define TCP_SYN (1 << 1)
#define TCP_RESET (1 << 2)
#define TCP_ACK (1 << 4)
#define TCP_FLAGS_MASK (TCP_FIN|TCP_SYN|TCP_RESET|TCP_ACK)
#define TCP_OFFSET_MASK 0xF000
#define ENCODE_TCP_OFFSET(sz) (((sz)/4) << 12)
#define DECODE_TCP_OFFSET(f) ((((f) & TCP_OFFSET_MASK) >> 12) * 4)

#define TCP_OPT_NOP 1
#define TCP_OPT_MSS 2

#define TCP_DEFAULT_WINDOW 0x4000 // about 10-11 packets
#define TCP_DEFAULT_MSS 1220 // min IPv6 MTU of 1280 - 40 IPv6 header & 20 TCP header
#define TCP_DEFAULT_ISS 0x10000000
#define TCP_RETRANSMIT_TIMEOUT MS_TO_TICKS(15*1000)
#define TCP_RETRANSMIT_TRIES 3

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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ETH_IP6 0x86DD

#define IP_TCP_NEW -1
#define IP_NDP_SOLICIT -2
#define IP_PING -3
#define IP_TCP 6
#define IP_UDP 11
#define IP_ICMP6 58

// ticks are in 100 ms chunks
#define TICK_MINUTE_SHIFT 9
#define RETRY_TICKS 2
#define DROP_TICKS 100

struct ipv6_addr {
	uint64_t high;
	uint64_t low;
};

struct addr_pair {
	uint64_t local_mac;
    uint64_t remote_mac;
	struct ipv6_addr local;
	struct ipv6_addr remote;
    uint16_t local_port;
    uint16_t remote_port;
    int type;
};

struct tcp_conn {
	struct eth_iface *ei;
    struct addr_pair ap;

	// data for the send side
    const char *next_out;		// next data for where the remote is up to
    int avail_out;				// data left for where the remote is up to
	bool fin_sent;				// whether we've sent a FIN
	bool fin_confirmed;			// whether the data with the FIN has been acked
	uint8_t dupacks;			// count of number of duplicate acknowledgments used for dupack retransmission
    uint32_t ack_received;		// where the remote is up to
    uint32_t seq_sent;			// where the send is up to
    uint32_t send_window;		// current send window as requested by remote

	// data for the receive side
	char *next_in;				// next buffer location for incoming data
	int avail_in;				// buffer room left for incoming data
	bool fin_received;			// whether we've received a FIN
	uint32_t seq_received;		// where we've received up to
    uint32_t ack_sent;			// where we've sent acks up to

	// timeout
    uint32_t last_rx_tick;		// tick when we last received a packet
};

static inline bool addr_equal(const struct addr_pair *a, const struct addr_pair *b) {
    return a->local.high == b->local.high
        && a->local.low == b->local.low
        && a->remote.high == b->remote.high
        && a->remote.low == b->remote.low
        && a->local_port == b->local_port
        && a->remote_port == b->remote_port;
}

int process_echo_request(struct eth_iface *ei, const struct addr_pair *ap, const struct ipv6_addr *addr, size_t addrn, void *data, uint16_t sz);
int process_neighbor_solicitation(struct eth_iface *ei, const struct addr_pair *ap, const struct ipv6_addr *addr, size_t addrn);
int process_tick(struct tcp_conn *c, uint32_t tick);
int process_ethernet(struct eth_iface *ei, void **pdata, uint16_t *psz, struct addr_pair *ap);
int accept_syn_cookie(struct tcp_conn *c, struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz);
int send_syn_cookie(struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz);
int process_tcp(struct tcp_conn *c, uint32_t tick, void *data, uint16_t sz);
int send_tcp(struct tcp_conn *c);
int send_reset(struct eth_iface *ei, const struct addr_pair *ap);
int abort_connection(struct tcp_conn *c);

extern char tcp_cookie_key[32];

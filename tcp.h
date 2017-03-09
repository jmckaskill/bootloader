#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IP_TCP_NEW 0
#define IP_TCP 6
#define IP_UDP 11
#define IP_ICMP6 58

// ticks are in 100 ms chunks
#define TICK_MINUTE_SHIFT 9
#define RETRY_TICKS 2
#define DROP_TICKS 100

struct addr_pair {
    uint64_t remote_mac;
    uint64_t local_ip[2];
    uint64_t remote_ip[2];
    uint16_t local_port;
    uint16_t remote_port;
    uint8_t type;
};

struct tcp_conn {
    bool connected;

    struct addr_pair ap;
    char *next_out;
    int avail_out;
    char *next_in;
    int avail_in;

    uint32_t last_ack_received;
    uint32_t cur_seq_sent;
    uint32_t send_window;

    uint32_t last_ack_sent;

    uint32_t last_rx_tick;
};

static inline bool addr_equal(const struct addr_pair *a, const struct addr_pair *b) {
    return a->local_ip[0] == b->local_ip[0]
        && a->local_ip[1] == b->local_ip[1]
        && a->remote_ip[0] == b->remote_ip[1]
        && a->remote_ip[1] == b->remote_ip[1]
        && a->local_port == b->local_port
        && a->remote_port == b->remote_port;
}

int process_tick(struct tcp_conn *c, uint32_t tick);
int process_ethernet(const void **pdata, int *plen, struct addr_pair *ap);
int accept_syn_cookie(struct tcp_conn *c, struct addr_pair *ap, const void *data, int sz);
int send_syn_cookie(struct addr_pair *ap, const void *data, int sz);
int process_tcp(struct tcp_conn *c, const void *data, int sz);
int send_tcp(struct tcp_conn *c);
int send_reset(const struct addr_pair *ap);
int abort_connection(struct tcp_conn *c);

extern char *get_tx_buffer();
extern int send_tx_buffer();
extern char tcp_cookie_key[32];

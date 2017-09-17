#include "tcp.h"
#include "../mdns/endian.h"
#include <stddef.h>

static struct tcp_connection *g_open;

#define MAX_CONNECTIONS 8
static struct tcp_connection g_connections[MAX_CONNECTIONS];

void init_tcp() {
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		g_connections[i].open = 0;
    }
    g_open = NULL;
}

static struct tcp_connection *new_connection() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        struct tcp_connection *c = &g_connections[i];
        if (!c->open) {
            return c;
        }
    }

    return NULL;
}

static struct tcp_connection *find_connection(struct ip6_header *ip, struct tcp_header *tcp) {
	uint16_t src = big_16(&tcp->src_port);
	uint16_t dst = big_16(&tcp->dst_port);
    
    for (struct tcp_connection *c = g_open; c != NULL; c = c->next) {
		if (c->local_port == dst && c->remote_port == src && ip6_equals(&ip->ip6_src, &c->remote_addr)) {
			return c;
		}
	}

	return NULL;
}

static void send_tcp_frame(struct tcp_header *tcp, int datasz) {
    struct ip6_header *ip = (struct ip6_header*) (tcp-1);
	int tcpsz = datasz + sizeof(*tcp);
	write_big_16(tcp->checksum, ones_checksum(ip, tcpsz));
	send_eth_frame(tcpsz);
}

static void send_reset(const mac_addr_t *mac, const ip6_addr_t *ip, uint16_t remote_port, uint16_t local_port) {
	struct ip6_header *rep = new_eth_frame(mac, ip, IP6_TCP);
	if (!rep) {
		return NULL;
	}

	struct tcp_header *tcp = (struct tcp_header*) (rep+1);
	write_big_16(tcp->src_port, local_port);
	write_big_16(tcp->dst_port, remote_port);
	write_big_32(tcp->seq_num, 0);
	write_big_32(tcp->ack_num, 0);
	write_big_16(tcp->flags, TCP_RESET | ENCODE_TCP_OFFSET(sizeof(*tcp)));
	write_big_16(tcp->window, 0);
	write_big_16(tcp->checksum, 0);
	write_big_16(tcp->urgent, 0);
	
	send_tcp_frame(tcp, 0);
}

static struct tcp_header *new_tcp_frame(struct tcp_connection *c) {
	struct ip6_header *ip = new_eth_frame(&c->remote_mac, &c->remote_addr, IP6_TCP);
	if (!ip) {
		return NULL;
	}

	struct tcp_header *tcp = (struct tcp_header*) (ip+1);
	write_big_16(tcp->src_port, c->local_port);
	write_big_16(tcp->dst_port, c->remote_port);
	write_big_32(tcp->seq_num, (uint32_t) (c->local_seq + c->sent));
	write_big_32(tcp->ack_num, c->remote_seq);
	write_big_16(tcp->flags, TCP_ACK | ENCODE_TCP_OFFSET(sizeof(*tcp)));
	write_big_16(tcp->window, TCP_DEFAULT_WINDOW);
	write_big_16(tcp->checksum, 0);
	write_big_16(tcp->urgent, 0);

	return tcp;
}

static void accept_tcp(struct tcp_connection *c, struct ip6_header *ip, struct tcp_header *tcp) {
	copy_mac(&c->remote_mac, &ip->eth_src);
	copy_ip6(&c->remote_addr, &ip->ip6_src);
	c->remote_port = big_16(tcp->src_port);
	c->remote_seq = big_32(tcp->seq_num);
	c->remote_mss = TCP_DEFAULT_MSS;
	c->remote_window = big_16(tcp->window);
	c->local_seq = 0;

	// process TCP options
	uint8_t *opt = (uint8_t*) (tcp+1);
    uint8_t *end = (uint8_t*) tcp + DECODE_TCP_OFFSET(big_16(tcp->flags));
	while (opt < end && *opt) {
		if (*opt == TCP_OPT_NOP) {
			opt++;
			continue;
		}

		uint8_t len = opt[1];
		if (opt + len > end) {
			break;
		}

		switch (opt[0]) {
		case TCP_OPT_MSS:
			if (len == 2) {
				c->remote_mss = big_16(opt+2);
			}
			break;
		}
		opt += len;
	}

	// send the SYN+ACK reply

	struct tcp_header *rep = new_tcp_frame(c);
	if (!rep) {
		return;
	}

	c->open = 1;
	c->connected = 0;
	c->in_finished = 0;
    c->out_finished = 0;

    c->next = g_open;
    c->prev = NULL;
    g_open = c;

	opt = (uint8_t*) (rep+1);
	opt[0] = TCP_OPT_MSS;
	opt[1] = 4; // option length
	write_big_16(opt+2, TCP_DEFAULT_MSS);

	write_big_16(tcp->flags, TCP_SYN | TCP_ACK | ENCODE_TCP_OFFSET(sizeof(*tcp) + 4));
	send_tcp_frame(tcp, 4);
}

static void reset_tcp(struct tcp_connection *c) {
	if (c->open) {
		c->open = 0;
		c->connected = 0;
		send_reset(&c->remote_mac, &c->remote_addr, c->remote_port, c->local_port);
	}
}

static void process_connection(struct tcp_connection *c, struct tcp_header *tcp, const void *data, int sz) {
#if 0
// TODO
	uint32_t remote_seq = big_32(req->seq_num);
	if (remote_seq != c->remote_seq) {
		return NULL;
	}

	unsigned flags = big_16(req->flags);
	int hdrsz = DECODE_TCP_OFFSET(flags);
	if (hdrsz < sizeof(*req)) {
		reset_tcp(c);
		return NULL;
	}

	*msgsz -= hdrsz;
	c->remote_seq += *msgsz;
	
	uint32_t local_seq = big_32(req->ack_num) - c->seq_base;


	// send the ACK reply
	struct tcp_header *tcp = new_tcp_frame(c);
#endif
}

void process_tcp(struct ip6_header *ip, const void *msg, int sz) {
    if (!ip6_equals(&ip->ip6_dst, &g_my_ip) || sz < sizeof(struct tcp_header)) {
        return;
    }

    struct tcp_header *tcp = (struct tcp_header*) msg;
    unsigned flags = big_16(tcp->flags) & TCP_FLAGS_MASK;
    int hdrsz = DECODE_TCP_OFFSET(flags);
    int src_port = big_16(tcp->src_port);
    int dst_port = big_16(tcp->dst_port);

    if (hdrsz > sz || !src_port) {
        return;
    }

    if (!(flags & TCP_SYN)) {
        struct tcp_connection *c = find_connection(ip, tcp);
        if (c) {
            process_connection(c, tcp, (uint8_t*) tcp + hdrsz, sz - hdrsz);
        } else {
            send_reset(&ip->eth_src, &ip->ip6_src, src_port, dst_port);
        }

    } else if (should_accept_connection(ip, dst_port)) {
        struct tcp_connection *c = new_connection();
        if (c) {
            accept_tcp(c, ip, tcp);
        } else {
            // too many connections, ignore the SYN. hopefully we are ready for it
            // when it next comes around
        }
    } else {
        // someone trying to open a port we are not interested in
        send_reset(&ip->eth_src, &ip->ip6_src, src_port, dst_port);
    }
}

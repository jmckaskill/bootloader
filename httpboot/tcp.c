#include "tcp.h"
#include "hw/tick.h"
#include "hw/endian.h"
#include "hw/copy.h"
#include <stddef.h>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static struct tcp_connection *g_open;

#define MAX_CONNECTIONS 8
static struct tcp_connection g_connections[MAX_CONNECTIONS];
unsigned g_tcp_timeout;

void init_tcp() {
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		g_connections[i].open = 0;
    }
    g_open = NULL;
    g_tcp_timeout = tick_count() + UINT32_MAX / 2;
}

static void update_tcp_timeout(struct tcp_connection *conn, unsigned new_timeout) {
    unsigned old_timeout = conn->tx_timeout;
    conn->tx_timeout = new_timeout;

    if (g_tcp_timeout != old_timeout) {
        return;
    }

    if (!g_open) {
        g_tcp_timeout = tick_count() + UINT32_MAX / 2;
    }

    unsigned timeout = g_open->tx_timeout;

    for (struct tcp_connection *c = g_open->next; c != NULL; c = c->next) {
        if ((int) (c->tx_timeout - timeout) < 0) {
            timeout = c->tx_timeout;
        }
    }

    g_tcp_timeout = timeout;
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
	uint16_t src = big_16(tcp->src_port);
	uint16_t dst = big_16(tcp->dst_port);

    for (struct tcp_connection *c = g_open; c != NULL; c = c->next) {
		if (c->local_port == dst && c->remote_port == src && ip6_equals(&ip->ip6_src, &c->remote_addr)) {
			return c;
		}
	}

	return NULL;
}

static void close_connection(struct tcp_connection *c) {
    if (c->next) {
        c->next->prev = c->prev;
    }
    if (c->prev) {
        c->prev->next = c->next;
    }
    if (c == g_open) {
        g_open = c->next;
    }
    update_tcp_timeout(c, 0);
    c->open = 0;
}

static void send_tcp_frame(struct tcp_header *tcp, int datasz) {
    struct ip6_header *ip = (struct ip6_header*) tcp - 1;
	int tcpsz = datasz + sizeof(*tcp);
	write_big_16(tcp->checksum, ones_checksum(ip, tcpsz));
	send_ip6_frame(ip, tcpsz);
}

static void send_reset(const mac_addr_t *mac, const ip6_addr_t *ip, uint16_t remote_port, uint16_t local_port) {
	struct ip6_header *rep = new_ip6_frame(mac, ip, IP6_TCP);
	if (!rep) {
		return;
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

static struct tcp_header *new_tcp_frame(struct tcp_connection *c, uint16_t flags, int off) {
	struct ip6_header *ip = new_ip6_frame(&c->remote_mac, &c->remote_addr, IP6_TCP);
	if (!ip) {
		return NULL;
	}

	struct tcp_header *tcp = (struct tcp_header*) (ip+1);
	write_big_16(tcp->src_port, c->local_port);
	write_big_16(tcp->dst_port, c->remote_port);
	write_big_32(tcp->seq_num, c->tx_next + off);
	write_big_32(tcp->ack_num, c->rx_next);
	write_big_16(tcp->flags, flags | ENCODE_TCP_OFFSET(sizeof(*tcp)));
	write_big_16(tcp->window, TCP_DEFAULT_WINDOW);
	write_big_16(tcp->checksum, 0);
	write_big_16(tcp->urgent, 0);

	return tcp;
}

static void send_syn_ack(struct tcp_connection *c) {
	struct tcp_header *rep = new_tcp_frame(c, TCP_SYN|TCP_ACK, 0);
	if (rep) {
        send_tcp_frame(rep, 0);
    }
    update_tcp_timeout(c, tick_count() + TCP_RETRANSMIT_TIMEOUT);
}

static void send_tx_data(struct tcp_connection *c) {
    int off = 0;
    int winsz = min(c->tx_size, c->tx_window);
    const char *data = (char*) c->tx_data;

    do {
        int pktsz = min(winsz, TCP_DEFAULT_MSS);
        int fin = (c->tx_size == pktsz && c->finishing);
        struct tcp_header *tcp = new_tcp_frame(c, fin ? (TCP_FIN|TCP_ACK) : TCP_ACK, off);
        copy_aligned_32(tcp+1, data, pktsz);
        send_tcp_frame(tcp, pktsz);
        off += pktsz;
        data += pktsz;
        winsz -= pktsz;
    } while (winsz);

    update_tcp_timeout(c, tick_count() + TCP_RETRANSMIT_TIMEOUT);
}

static void accept_connection(struct tcp_connection *c, struct ip6_header *ip, struct tcp_header *tcp) {
    copy_mac(&c->remote_mac, &ip->eth_src);
	copy_ip6(&c->remote_addr, &ip->ip6_src);
    c->remote_port = big_16(tcp->src_port);
    c->local_port = big_16(tcp->dst_port);

    c->rx_next = big_32(tcp->seq_num) + 1;

    c->tx_next = TCP_DEFAULT_ISS;
	c->tx_window = big_16(tcp->window);
    c->tx_sent = 0;
    c->tx_timeout = 0;
    c->tx_retries = 0;

    c->tx_data = NULL;
    c->tx_size = 0;

    c->connected = 0;
    c->finishing = 0;

    c->open = 1;
    c->next = g_open;
    c->prev = NULL;
    g_open = c;

    send_syn_ack(c);
}

static void reset_connection(struct tcp_connection *c) {
    send_reset(&c->remote_mac, &c->remote_addr, c->remote_port, c->local_port);
    close_connection(c);
}

static void process_connection(struct tcp_connection *c, struct tcp_header *tcp, const void *data, int sz) {
    unsigned flags = big_32(tcp->flags);
    if (flags & TCP_RESET) {
        close_connection(c);
        return;
    }

    uint32_t ack = big_32(tcp->ack_num);
    int32_t txdelta = (int32_t) (ack - c->tx_next);

    if (txdelta < 0) {
        // old ack -> old message -> drop
        return;

    } else if (txdelta > c->tx_sent || ((txdelta != c->tx_size) && (txdelta % c->tx_window))) {
        // invalid or unreasonable ack
        // this protects us against slow acknowledgers, unalign acks, etc
        reset_connection(c);
        return;
    }

    // make sure the window is aligned to keep the tx_data aligned
    c->tx_window = big_16(tcp->window) & 3;
    if (c->tx_window == 0) {
        // not interested in zero sized windows
        reset_connection(c);
        return;
    }

    if (txdelta) {
        // the remote acknowledged some new data
        c->tx_sent -= (uint16_t) txdelta;
        c->tx_data = (char*) c->tx_data + txdelta;
        c->tx_size -= txdelta;
        c->tx_retries = 0;
    }

    uint32_t seq = big_32(tcp->seq_num);
    int32_t rxdelta = (int32_t) (seq - c->rx_next);

    if (rxdelta) {
        // out of order packet or dropped packet -> ignore
        // we'll send out the update window shortly which will
        // contain our current ack plus any new data
    } else {
        int callback = sz;
        c->rx_next += sz;

        if (!c->connected) {
            // we've finished the SYN-SYN/ACK-ACK sequence
            // we need to increment the ack number to acknowledge
            c->connected = 1;
            c->rx_next++;
            callback = 1;
        } else if (c->tx_data && !c->tx_size) {
            // we've ran out of data to send
            // give the user a chance to send more
            c->tx_data = NULL;
            callback = 1;
        }

        if (c->finishing && !(flags & TCP_FIN)) {
            // the client has acknowledged our close. we're done
            close_connection(c);
            return;
        } else if (!c->finishing && (flags & TCP_FIN)) {
            // the client has asked us to close
            // we need to increment the ack number to acknowledge
            // we'll still give the app a chance to send it's final data
            c->rx_next++;
            c->finishing = 1;
            callback = 1;
        }

        if (callback && process_tcp_data(c, data, sz)) {
            reset_connection(c);
            return;
        }
    }

    send_tx_data(c);
}

void process_tcp(struct ip6_header *ip, const void *msg, int sz) {
    if (!ip6_equals(&ip->ip6_dst, &g_my_ip) || sz < sizeof(struct tcp_header)) {
        return;
    }

    struct tcp_header *tcp = (struct tcp_header*) msg;
    unsigned flags = big_16(tcp->flags);
    int hdrsz = DECODE_TCP_OFFSET(flags);
    uint16_t src_port = big_16(tcp->src_port);
    uint16_t dst_port = big_16(tcp->dst_port);

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
            accept_connection(c, ip, tcp);
        } else {
            // too many connections, ignore the SYN. hopefully we are ready for it
            // when it next comes around
        }
    } else {
        // someone trying to open a port we are not interested in
        send_reset(&ip->eth_src, &ip->ip6_src, src_port, dst_port);
    }
}

void process_tcp_timeout(unsigned now) {
    for (struct tcp_connection *c = g_open; c != NULL; c = c->next) {
        if (++c->tx_retries == TCP_RETRANSMIT_TRIES) {
            reset_connection(c);
        } else if (c->connected) {
            send_tx_data(c);
        } else {
            send_syn_ack(c);
        }
    }
}

#include "tcp.h"
#include "endian.h"
#include "crc.h"
#include "eth.h"
#include <assert.h>
#include <stddef.h>
#include <bearssl.h>

#pragma pack(push)
#pragma pack(1)
struct eth_header {
    uint8_t dst[6];
    uint8_t src[6];
    uint8_t type[2];
};

struct eth_footer {
    uint8_t crc[4];
};

struct ip6_header {
	uint8_t class_version;
	uint8_t flow[3];
    uint8_t length[2];
    uint8_t next;
    uint8_t hop;
    uint8_t src[16];
    uint8_t dst[16];
};

struct tcp_header {
    uint8_t src[2];
    uint8_t dst[2];
    uint8_t seq[4];
    uint8_t ack[4];
	uint8_t offset_reserved;
	uint8_t flags;
    uint8_t window[2];
    uint8_t checksum[2];
    uint8_t urgent[2];
};
struct icmp6_header {
    uint8_t type;
    uint8_t code;
	uint8_t crc[2];
};
#pragma pack(pop)

#define IP6_VERSION_SHIFT 4
#define TCP_OFFSET_SHIFT 4

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PUSH 0x08
#define TCP_ACK 0x10
#define TCP_URGENT 0x20

static_assert(sizeof(struct eth_header) == 14, "padding");
static_assert(sizeof(struct eth_footer) == 4, "padding");
static_assert(sizeof(struct ip6_header) == 40, "padding");
static_assert(sizeof(struct tcp_header) == 20, "padding");
static_assert(sizeof(struct icmp6_header) == 4, "padding");

#define TCP_MAX_MTU (1280 - sizeof(struct tcp_header))
#define TCP_SEND_WINDOW (ETH_TX_QUEUE_SIZE * TCP_MAX_MTU)
#define TCP_RECV_WINDOW (ETH_RX_QUEUE_SIZE * TCP_MAX_MTU)

static void fill_tcp_checksum(struct tcp_header *tcp, const struct addr_pair *ap, uint16_t datasz) {
	struct ip6_header *ip = ((struct ip6_header*) tcp) - 1;
	write_big_16(tcp->checksum, 0);
	uint32_t sum = 0;

	// add the pseudo header
	uint8_t *p = (uint8_t*) ip->src;
	for (int i = 0; i < 16; i++) {
		sum += read_big_16(p + i*2);
	}
	sum += datasz;
	sum += IP_TCP;

	// add the tcp header
	p = (uint8_t*) tcp;
	for (int i = 0; i < datasz/2; i++) {
		sum += read_big_16(p + i*2);
	}

	// zero fill the last byte if unaligned
	if (datasz & 1) {
		sum += p[datasz] << 8;
	}

	uint16_t sum16 = ((uint16_t) sum) + ((uint16_t) (sum >> 16));
	write_big_16(tcp->checksum, ~sum16);
}

int process_ethernet(struct eth_iface *ei, void **pdata, uint16_t *psz, struct addr_pair *ap) {
    struct eth_header *eh = (struct eth_header*) *pdata;
    struct ip6_header *ip = (struct ip6_header*) (eh + 1);

    if (*psz < sizeof(struct eth_header) + sizeof(struct eth_footer) + sizeof(struct ip6_header)) {
        return -1;
    }

	uint64_t dst = read_big_48(eh->dst);
	if (dst != ei->local_mac) {
		return -1;
	}

	if (read_big_16(eh->type) != ETH_IP6 || (ip->class_version >> IP6_VERSION_SHIFT) != 6) {
        return -1;
    }

    char *sesdata = (char*) (ip + 1);
	uint16_t seslen = *psz - sizeof(struct eth_header) - sizeof(struct ip6_header);

    *pdata = sesdata;
	*psz -= seslen;

	ap->local_mac = dst;
    ap->remote_mac = read_big_48(eh->src);

    ap->remote_ip[0] = read_big_64(&ip->src[0]);
    ap->remote_ip[1] = read_big_64(&ip->src[8]);
    ap->local_ip[0] = read_big_64(&ip->dst[0]);
    ap->local_ip[1] = read_big_64(&ip->dst[8]);

    ap->type = ip->next;

    switch (ip->next) {
    case IP_TCP:
        if (seslen >= sizeof(struct tcp_header)) {
            struct tcp_header *tcp = (struct tcp_header*) sesdata;
			uint16_t chk = read_big_16(tcp->checksum);
			fill_tcp_checksum(tcp, ap, seslen);
			if (chk != read_big_16(tcp->checksum)) {
				return -1;
			}
            if ((tcp->flags & TCP_SYN) && (tcp->flags & (TCP_FIN | TCP_RST))) {
                return -1;
            }
            if ((tcp->offset_reserved >> TCP_OFFSET_SHIFT) * 4 > seslen) {
                return -1;
            }

            ap->remote_port = read_big_16(tcp->src);
            ap->local_port = read_big_16(tcp->dst);

            if (tcp->flags & TCP_SYN) {
                ap->type = IP_TCP_NEW;
            }
            return 0;
        }
        break;

    case IP_ICMP6:
        if (seslen >= sizeof(struct icmp6_header)) {
            struct icmp6_header *icmp = (struct icmp6_header*) sesdata;
            ap->local_port = icmp->type;
            ap->remote_port = icmp->code;
            return 0;
        }
        break;
    }

    return -1;
}

static struct eth_buffer *next_ip_buffer(struct eth_iface *ei, const struct addr_pair *ap, uint8_t next, uint16_t datasz) {
	struct eth_buffer *eb = eth_next_tx(ei, datasz + sizeof(struct eth_header) + sizeof(struct eth_footer) + sizeof(struct ip6_header));
	if (eb) {
		struct eth_header *eh = (struct eth_header*) eb->data;
		struct ip6_header *ip = (struct ip6_header*) (eh + 1);

		write_big_48(eh->dst, ap->remote_mac);
		write_big_48(eh->src, ei->local_mac);
		write_big_16(eh->type, ETH_IP6);

		ip->class_version = 6 << IP6_VERSION_SHIFT;
		memset(ip->flow, 0, sizeof(ip->flow));
		write_big_16(ip->length, datasz);
		ip->next = next;
		ip->hop = 255;
		write_big_64(&ip->src[0], ap->local_ip[0]);
		write_big_64(&ip->src[8], ap->local_ip[1]);
		write_big_64(&ip->dst[0], ap->remote_ip[0]);
		write_big_64(&ip->dst[8], ap->remote_ip[1]);
	}

	return eb;
}

#define TCP_OFF (sizeof(struct eth_header) + sizeof(struct ip6_header))
#define TCP_BUFSZ (TCP_OFF + sizeof(struct tcp_header) + sizeof(struct eth_footer))

static struct eth_buffer *next_tcp_buffer(struct eth_iface *ei, const struct addr_pair *ap, uint32_t seq, uint16_t datasz) {
	struct eth_buffer *eb = next_ip_buffer(ei, ap, IP_TCP, datasz + sizeof(struct tcp_header));
	if (eb) {
		struct tcp_header *tcp = (struct tcp_header*) (eb->data + TCP_OFF);

		write_big_16(tcp->src, ap->local_port);
		write_big_16(tcp->dst, ap->remote_port);
		write_big_32(tcp->seq, seq);
		write_big_32(tcp->ack, 0);
		tcp->offset_reserved = (sizeof(struct tcp_header) / 4) << TCP_OFFSET_SHIFT;
		tcp->flags = 0;
		write_big_16(tcp->window, TCP_RECV_WINDOW);
		write_big_16(tcp->checksum, 0);
		write_big_16(tcp->urgent, 0);
	}

	return eb;
}


int send_reset(struct eth_iface *ei, const struct addr_pair *ap) {
	struct eth_buffer *eb = next_tcp_buffer(ei, ap, 0, 0);
	if (!eb) {
		return -1;
	}
    struct tcp_header *tcp = (struct tcp_header*) (eb->data + TCP_OFF);
    tcp->flags = TCP_RST;
	fill_tcp_checksum(tcp, ap, 0);
	eth_sched_tx(ei, eb);
	return 0;
}

int abort_connection(struct tcp_conn *c) {
	c->ei = NULL;
    return send_reset(c->ei, &c->ap);
}

static int do_send_tcp(struct tcp_conn *c, uint32_t ack_to_send) {
    for (;;) {
        int tosend = (int32_t) (c->last_ack_received + c->send_window - c->cur_seq_sent);

        if (tosend > c->avail_out) {
            tosend = c->avail_out;
        } else if (tosend < 0) {
            tosend = 0;
        }

        if (tosend > 1280) {
            tosend = 1280;
        }

        if (!tosend && c->last_ack_sent == ack_to_send) {
            return 0;
        }

        struct eth_buffer *eb = next_tcp_buffer(c->ei, &c->ap, c->cur_seq_sent, tosend);
		if (!eb) {
			return 0;
		}

        struct tcp_header *tcp = (struct tcp_header*) (eb->data + TCP_OFF);
        char *msg = (char*) (tcp + 1);

        memcpy(msg, c->next_out, tosend);

		tcp->flags = TCP_ACK;
        write_big_32(tcp->ack, ack_to_send);
		fill_tcp_checksum(tcp, &c->ap, tosend);
		eth_sched_tx(c->ei, eb);

        c->cur_seq_sent += tosend;
        c->last_ack_sent = ack_to_send;
    }
}

int send_tcp(struct tcp_conn *c) {
    return do_send_tcp(c, c->last_ack_sent);
}

#define ADDR_PAIR_PACKED (16+16+2+2+1)
static_assert(offsetof(struct addr_pair, type) + sizeof(((struct addr_pair*)0)->type) - offsetof(struct addr_pair, local_ip) == ADDR_PAIR_PACKED, "padding needs to be at the end for sha1 to work");

static uint32_t generate_syn_cookie(struct addr_pair *ap, uint32_t minute) {
    // standard for cookies is for the top 5 bits to indicate the minute
    // the cookie was generated in
    uint32_t t = minute & 0x1F;
    uint32_t m = 0;

    // store this in here temporarily so we can make a single sha1 call
    uint8_t type = ap->type;
    ap->type = (uint8_t) t;
    
    unsigned char h[br_sha1_SIZE];
	br_sha1_context ctx;
	br_sha1_init(&ctx);
	br_sha1_update(&ctx, ap, ADDR_PAIR_PACKED);
    br_sha1_update(&ctx, tcp_cookie_key, sizeof(tcp_cookie_key));
	br_sha1_out(&ctx, h);

    ap->type = type;

    return (t << 27) 
         | (m << 24) 
         | (((uint32_t) h[0]) << 16)
         | (((uint32_t) h[1]) << 8)
         | (((uint32_t) h[2]));
}

int do_recv_tcp(struct tcp_conn *c, void *data, uint16_t sz) {
	struct tcp_header *tcp = (struct tcp_header*) data;
	uint16_t off = (tcp->offset_reserved >> TCP_OFFSET_SHIFT) * 4;
	char *msg = (char*) tcp + off;
	uint16_t mlen = sz - off;

	if (mlen > c->avail_in) {
		return abort_connection(c);
	}

	memcpy(c->next_in, msg, mlen);
	c->next_in += mlen;
	c->avail_in -= mlen;

	return do_send_tcp(c, c->last_ack_sent + mlen);
}

int accept_syn_cookie(struct tcp_conn *c, struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz) {
    struct tcp_header *tcp = (struct tcp_header*) data;

    uint32_t now_minute = tick >> TICK_MINUTE_SHIFT;
    uint32_t ack = read_big_32(tcp->ack);
    uint32_t syn_minute = (ack >> 27) & 0x1F;
    uint32_t cookie = generate_syn_cookie((struct addr_pair*) ap, syn_minute);

    // the tick check catches if the syn was more than 64 seconds ago
    // or is from the future because it's unsigned
    if (now_minute - syn_minute > 1 || cookie != ack) {
        return -1;
    }

	c->ei = ei;
    c->ap = *ap;
    c->last_ack_received = ack;
    c->cur_seq_sent = ack;
	c->last_ack_sent = read_big_32(tcp->seq);
    c->send_window = TCP_RECV_WINDOW;
	c->last_rx_tick = tick;

    return do_recv_tcp(c, data, sz);
}

int send_syn_cookie(struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz) {
	struct tcp_header *tcpin = (struct tcp_header*) data;

    uint32_t cookie = generate_syn_cookie((struct addr_pair *) ap, tick >> TICK_MINUTE_SHIFT);

	struct eth_buffer *eb = next_tcp_buffer(ei, ap, cookie, 0);
	if (!eb) {
		return -1;
	}
    struct tcp_header *tcp = (struct tcp_header*) (eb->data + TCP_OFF);

	tcp->flags = TCP_ACK | TCP_SYN;
	memcpy(tcp->ack, tcpin->seq, sizeof(tcp->ack));
	fill_tcp_checksum(tcp, ap, 0);
	eth_sched_tx(ei, eb);
	return 0;
}

int process_tcp(struct tcp_conn *c, uint32_t tick, void *data, uint16_t sz) {
    struct tcp_header *tcp = (struct tcp_header*) data;
    c->last_rx_tick = tick;

    if (tcp->flags & TCP_ACK) {
		// TODO check that the ack was valid
        int sent = (int) (tcp->ack - c->last_ack_received);
        c->avail_out -= sent;
        c->next_out += sent;
        c->last_ack_received = read_big_32(tcp->ack);
    }

	// todo check that the seq is valid
    uint32_t seq = read_big_32(tcp->seq);

	if (seq != c->last_ack_sent) {
        // we don't handle out of order packets
		return 0;
	}
	
	return do_recv_tcp(c, data, sz);
}

int process_tick(struct tcp_conn *c, uint32_t tick) {
    uint32_t t = tick - c->last_rx_tick;
    if (t < RETRY_TICKS) {
        return 0;
    } else if (t > DROP_TICKS) {
        return abort_connection(c);
    }

    // set the ack to something else to force an ack resend
    uint32_t ack = c->last_ack_sent--;
    return do_send_tcp(c, ack);
}

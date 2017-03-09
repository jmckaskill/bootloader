#include "tcp.h"
#include "endian.h"
#include <assert.h>

#pragma pack(push)
#pragma pack(1)
struct eth_header {
    uint8_t mac_dst[6];
    uint8_t mac_src[6];
    uint8_t length[2];
};

struct eth_footer {
    uint8_t crc[4];
};

struct ip6_header {
    uint8_t version : 4;
    uint8_t class;
    uint8_t flow_hi : 4;
    uint8_t flow[2];
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
    uint8_t offset : 4;
    uint8_t reserved : 6;
    uint8_t has_urgent : 1; // not used
    uint8_t has_ack : 1;
    uint8_t has_push : 1; // not used
    uint8_t has_rst : 1;
    uint8_t has_syn : 1;
    uint8_t has_fin : 1;
    uint8_t window[2];
    uint8_t checksum[2];
    uint8_t urgent[2];
};
struct icmp6_header {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};
#pragma pack(pop)

static_assert(sizeof(struct eth_header) == 14, "padding");
static_assert(sizeof(struct eth_footer) == 4, "padding");
static_assert(sizeof(struct ip6_header) == 40, "padding");
static_assert(sizeof(struct tcp_header) == 20, "padding");
static_assert(sizeof(struct icmp6_header) == 4, "padding");

static uint32_t g_tick;

int process_ethernet(const void **pdata, int *plen, struct addr_pair *ap) {
    struct eth_header *eh = (struct eth_header*) *pdata;
    int ethlen = *plen;
    if (ethlen < sizeof(struct eth_header) + sizeof(struct eth_footer)) {
        return -1;
    }
    int iplen = read_big_16(eh->length);
    if (iplen + sizeof(struct eth_header) + sizeof(struct eth_footer)) > ethlen) {
        return -1;
    }
    char *ipdata = (char*) (eh + 1);
    struct eth_footer *ef = (struct eth_footer*) (ipdata + iplen);
    if (calc_crc(eh) != read_big_32(ef->crc)) {
        return -1;
    }

    struct ip6_header *ip = (struct ip6_header*) ipdata;
    if (ip->version != 6) {
        return -1;
    }

    char *sesdata = (char*) (ip + 1);
    int seslen = iplen - sizeof(*ip);

    *pdata = sesdata;
    *plen = seslen;

    ap->remote_mac = read_big_48(eh->src);

    ap->src_ip[0] = read_big_64(&ip->src[0]);
    ap->src_ip[1] = read_big_64(&ip->src[8]);
    ap->dst_ip[0] = read_big_64(&ip->dst[0]);
    ap->dst_ip[1] = read_big_64(&ip->dst[8]);

    ap->type = ip->next;

    switch (ip->next) {
    case IP_TCP:
        if (seslen >= sizeof(struct tcp_header)) {
            struct tcp_header *tcp = (struct tcp_header*) sesdata;
            if (tcp->has_syn && (tcp->has_fin || tcp->has_rst)) {
                return -1;
            }
            if (tcp->offset * 4 > seslen) {
                return -1;
            }

            ap->src_port = read_big_16(tcp->src);
            ap->dst_port = read_big_16(tcp->dst);

            if (tcp->has_syn) {
                ap->type = IP_TCP_NEW;
            }
            return 0;
        }
        break;

    case IP_ICMP6:
        if (seslen >= sizeof(struct icmp6_header)) {
            struct icmp6_header *icmp = (struct icmp6_header*) sesdata;
            ap->dst_port = icmp->type;
            ap->src_port = icmp->code;
            return 0;
        }
        break;
    }

    return -1;
}

static int send_ip(char *buf, int sz, const struct addr_pair *ap) {
    struct eth_header *eh = (struct eth_header*) buf;
    struct ip6_header *ip = (struct ip6_header*) (eh + 1);
    struct eth_footer *ef = (struct eth_footer*) (buf + sz) - 1;

    write_big_48(eh->mac_dst, ap->remote_mac);
    write_big_16(eh->length, (uint16_t) (sz - sizeof(*eh) - sizeof(*ef)));

    ip->version = 6;
    write_big_16(ip->length, (uint16_t) (sz - sizeof(*eh) - sizeof(*ip) - sizeof(*ef)));
    ip->hop = 255;
    write_big_64(&ip->src[0], ap->local_ip[0]);
    write_big_64(&ip->src[8], ap->local_ip[1]);
    write_big_64(&ip->dst[0], ap->remote_ip[0]);
    write_big_64(&ip->dst[8], ap->remote_ip[1]);

    write_big_32(ef->crc, calc_eth_crc(buf, sz));

    return send_tx_buffer();
}

static char *get_tcp_buffer(const struct addr_pair *ap, uint32_t seq) {
    char *buf = get_tx_buffer();
    struct eth_header *eh = (struct eth_header*) buf;
    struct ip6_header *ip = (struct ip6_header*) (eh + 1);
    struct tcp_header *tcp = (struct tcp_header*) (ip + 1);
    
    memset(buf, 0, sizeof(*eh) + sizeof(*ip) + sizeof(*tcp));
    
    write_big_16(tcp->dst, ap->remote_port);
    write_big_16(tcp->src, ap->local_port);
    write_big_32(tcp->seq, seq);
    tcp->offset = sizeof(struct tcp_header) / 4;
    write_big_16(tcp->window, DEFAULT_SEND_WINDOW);

    ip->next = IP_TCP;
    return buf;
}

#define TCP_OFFSET (sizeof(struct eth_header) + sizeof(struct ip6_header))
#define TCP_BUFSZ (TCP_OFSET + sizeof(struct tcp_header) + sizeof(strcut eth_footer))

int send_reset(const struct addr_pair *ap) {
    char *buf = get_tcp_buffer(ap, 0);
    struct tcp_buffer *tcp = (struct tcp_buffer*) (buf + TCP_OFF);
    tcp->has_rst = 1;
    calc_tcp_checksum(tcp, 0);
    return send_ip(ap, TCP_BUFSZ);
}

int abort_connection(struct tcp_conn *c) {
    c->connected = false;
    return send_reset(&c->ap);
}

static int do_send_tcp(struct tcp_conn *c, uint32_t ack) {
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

        if (!tosend && c->last_ack_sent == ack) {
            return 0;
        }

        char *buf = get_tcp_buffer(&c->ap, c->cur_seq_sent);
        struct tcp_buffer *tcp = (struct tcp_buffer*) (buf + TCP_OFF);

        char *msg = (char*) (tcp + 1);
        memcpy(msg, c->next_out, tosend);

        if (c->last_ack_sent != ack) {
            tcp->has_ack = 1;
            write_big_32(tcp->ack, ack);
        }
        calc_tcp_checksum(tcp, tosend);

        int err = send_ip(buf, end - buf, ap);
        if (err) {
            return err;
        }

        tcp->cur_seq_sent += tosend;
        c->last_ack_sent = ack;
    }
}

int send_tcp(struct tcp_conn *c) {
    return do_send_tcp(c, c->last_ack_sent);
}

#define ADDR_PAIR_PACKED (16+16+2+2+1)
static_assert(offsetof(struct addr_pair, type) - offsetof(struct addr_pair, remote_mac) == ADDR_PAIR_PACKED, "padding needs to be at the end for sha1 to work");

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
	br_sha1_out(&ctx, hash);

    ap->type = type;

    return (t << 27) 
         | (m << 24) 
         | (((uint32_t) h[0]) << 16)
         | (((uint32_t) h[1]) << 8)
         | (((uint32_t) h[2]));
}

int accept_syn_cookie(struct tcp_conn *c, struct addr_pair *ap, const void *data, int sz) {
    struct tcp_header *tcp = (struct tcp_data*) data;

    uint32_t now_minute = g_tick >> TICK_MINUTE_SHIFT;
    uint32_t ack = read_big_32(tcp->ack);
    uint32_t syn_minute = (ack >> 27) & 0x1F;
    uint32_t cookie = generate_syn_cookie(ap, syn_minute);

    // the tick check catches if the syn was more than 64 seconds ago
    // or is from the future because it's unsigned
    if (now_minute - syn_minute > 1 || cookie != read_big_32(ack)) {
        return -1;
    }

    c->ap = *ap;
    c->last_ack_received = ack;
    c->cur_seq_sent = ack;
    c->send_window = DEFAULT_SEND_WINDOW;

    return do_send_tcp(c);
}

int send_syn_cookie(struct addr_pair *ap, const void *data, int sz) {
    (void) data;
    (void) sz;

    uint32_t cookie = generate_syn_cookie(ap, g_tick >> TICK_MINUTE_SHIFT);

    char *buf = get_tcp_buffer(ap, cookie);
    struct tcp_buffer *tcp = (struct tcp_buffer*) (buf + TCP_OFF);

    tcp->has_syn = 1;
    tcp->has_ack = 1;
    
    calc_tcp_checksum(tcp, 0);

    return send_ip(buf, end - buf, ap);
}

int process_tcp(struct tcp_conn *c, const void *data, int sz) {
    struct tcp_header *tcp = (struct tcp_data*) data;
    c->last_rx_tick = g_tick;

    if (tcp->has_ack) {
        int sent = (int) (tcp->ack - c->last_ack_received);
        c->avail_out -= sent;
        c->next_out += sent;
        c->last_ack_received = tcp->ack;
    }

    uint32_t seq = read_little_32(tcp->seq);

    if (seq == c->last_ack_sent) {
        char *msg = (char*) tcp + (tcp->offset * 4);
        int mlen = sz - (tcp->offset * 4);
        
        if (mlen > c->avail_in) {
            return abort_connection(c);
        }

        memcpy(c->next_in, msg, mlen);
        c->next_in += mlen;
        c->avail_in -= mlen;
    } else {
        // we don't handle out of order packets
        seq = c->last_ack_sent;
    }

    return do_send_tcp(c, seq);
}

int process_tick(struct tcp_conn *c, uint32_t tick) {
    g_tick = tick;

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

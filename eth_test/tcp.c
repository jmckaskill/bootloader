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
	uint8_t checksum[2];
};
struct neighbor_solicitation {
	uint32_t reserved;
	uint8_t address[2];
};
struct neighbor_advertisement {
	uint8_t flags[4];
	uint8_t address[16];
	uint8_t target_type;
	uint8_t target_len;
	uint8_t target_mac[6];
};
#pragma pack(pop)

#define IP6_VERSION_SHIFT 4
#define TCP_OFFSET_SHIFT 4

#define ICMP6_NEIGHBOR_SOLICITATION 0x87
#define ICMP6_NEIGHBOR_ADVERTISEMENT 0x88
#define ICMP6_ECHO_REQUEST 0x80
#define ICMP6_ECHO_RESPONSE 0x81

#define NDP_ROUTER		0x80000000
#define NDP_SOLICITED	0x40000000
#define NDP_OVERRIDE	0x20000000

#define NDP_OPT_TARGET_TYPE 2
#define NDP_OPT_TARGET_LEN 1

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PUSH 0x08
#define TCP_ACK 0x10
#define TCP_URGENT 0x20

static_assert(sizeof(struct eth_header) == 14, "padding");
static_assert(sizeof(struct ip6_header) == 40, "padding");
static_assert(sizeof(struct tcp_header) == 20, "padding");
static_assert(sizeof(struct icmp6_header) == 4, "padding");

#define TCP_MAX_MTU (1280 - sizeof(struct tcp_header))
#define TCP_SEND_WINDOW (ETH_TX_QUEUE_SIZE * TCP_MAX_MTU * 2)
#define TCP_RECV_WINDOW (ETH_RX_QUEUE_SIZE * TCP_MAX_MTU * 2)

static uint16_t ones_checksum(struct ip6_header *ip, uint16_t ipdatasz) {
	uint32_t sum = 0;

	// add the pseudo header
	uint8_t *p = (uint8_t*) ip->src;
	for (int i = 0; i < 16; i++) {
		sum += read_big_16(p + i * 2);
	}
	sum += ipdatasz;
	sum += ip->next;

	// add the tcp header
	p = (uint8_t*) (ip+1);
	for (int i = 0; i < ipdatasz / 2; i++) {
		sum += read_big_16(p + i * 2);
	}

	// zero fill the last byte if unaligned
	if (ipdatasz & 1) {
		sum += p[ipdatasz] << 8;
	}

	uint16_t sum16 = ((uint16_t) sum) + ((uint16_t) (sum >> 16));
	return ~sum16;
}

static void fill_tcp_checksum(struct tcp_header *tcp, uint16_t datasz) {
	write_big_16(tcp->checksum, 0);
	uint16_t sum = ones_checksum(((struct ip6_header*) tcp) - 1, datasz + sizeof(*tcp));
	write_big_16(tcp->checksum, sum);
}

static void fill_icmp6_checksum(struct icmp6_header *icmp, uint16_t datasz) {
	write_big_16(icmp->checksum, 0);
	uint16_t sum = ones_checksum(((struct ip6_header*) icmp) - 1, datasz + sizeof(*icmp));
	write_big_16(icmp->checksum, sum);
}


int process_ethernet(struct eth_iface *ei, void **pdata, uint16_t *psz, struct addr_pair *ap) {
    struct eth_header *eh = (struct eth_header*) *pdata;
    struct ip6_header *ip = (struct ip6_header*) (eh + 1);

    if (*psz < sizeof(struct eth_header) + sizeof(struct ip6_header)) {
        return -1;
    }

	if (read_big_16(eh->type) != ETH_IP6 || (ip->class_version >> IP6_VERSION_SHIFT) != 6) {
        return -1;
    }

    char *sesdata = (char*) (ip + 1);
	uint16_t seslen = *psz - sizeof(struct eth_header) - sizeof(struct ip6_header);

    *pdata = sesdata;
	*psz = seslen;

	ap->local_mac = read_big_48(eh->dst);
    ap->remote_mac = read_big_48(eh->src);

    ap->remote.high = read_big_64(&ip->src[0]);
    ap->remote.low = read_big_64(&ip->src[8]);
    ap->local.high = read_big_64(&ip->dst[0]);
    ap->local.low = read_big_64(&ip->dst[8]);

    ap->type = ip->next;

    switch (ip->next) {
    case IP_TCP:
        if (seslen >= sizeof(struct tcp_header)) {
            struct tcp_header *tcp = (struct tcp_header*) sesdata;

			uint16_t sum = read_big_16(tcp->checksum);
			write_big_16(tcp->checksum, 0);
			if (sum != ones_checksum(ip, seslen)) {
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

			uint16_t sum = read_big_16(icmp->checksum);
			write_big_16(icmp->checksum, 0);
			uint16_t got = ones_checksum(ip, seslen);
			if (sum != got) {
				return -1;
			}

            ap->local_port = icmp->type;
            ap->remote_port = icmp->code;
			
			if (icmp->type == ICMP6_NEIGHBOR_SOLICITATION && icmp->code == 0) {
				ap->type = IP_NDP_SOLICIT;
			} else if (icmp->type == ICMP6_ECHO_REQUEST && icmp->code == 0) {
				ap->type = IP_PING;
			}

            return 0;
        }
        break;
    }

    return -1;
}

static struct eth_buffer *next_ip_buffer(struct eth_iface *ei, const struct addr_pair *ap, uint8_t next, uint16_t datasz) {
	struct eth_buffer *eb = eth_next_tx(ei, datasz + sizeof(struct eth_header) + sizeof(struct ip6_header));
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
		ip->hop = 64;
		write_big_64(&ip->src[0], ap->local.high);
		write_big_64(&ip->src[8], ap->local.low);
		write_big_64(&ip->dst[0], ap->remote.high);
		write_big_64(&ip->dst[8], ap->remote.low);
	}

	return eb;
}

#define SES_OFF (sizeof(struct eth_header) + sizeof(struct ip6_header))
#define TCP_BUFSZ (SES_OFF + sizeof(struct tcp_header))

static struct eth_buffer *next_tcp_buffer(struct eth_iface *ei, const struct addr_pair *ap, uint16_t datasz) {
	struct eth_buffer *eb = next_ip_buffer(ei, ap, IP_TCP, datasz + sizeof(struct tcp_header));
	if (eb) {
		struct tcp_header *tcp = (struct tcp_header*) (eb->data + SES_OFF);

		write_big_16(tcp->src, ap->local_port);
		write_big_16(tcp->dst, ap->remote_port);
		write_big_32(tcp->seq, 0);
		write_big_32(tcp->ack, 0);
		tcp->offset_reserved = (sizeof(struct tcp_header) / 4) << TCP_OFFSET_SHIFT;
		tcp->flags = 0;
		write_big_16(tcp->window, TCP_RECV_WINDOW);
		write_big_16(tcp->checksum, 0);
		write_big_16(tcp->urgent, 0);
	}

	return eb;
}

static bool is_multicast(const struct ipv6_addr *addr) {
	return (addr->high >> 56) == 0xFF;
}

static int get_unicast(const struct ipv6_addr *addr, size_t addrn, struct ipv6_addr *to) {
	for (size_t i = 0; i < addrn; i++) {
		if (addr[i].low == to->low && addr[i].high == to->high) {
			if (is_multicast(to)) {
				*to = addr[0];
			}
			return 0;
		}
	}
	return -1;
}



int process_echo_request(struct eth_iface *ei, const struct addr_pair *ap, const struct ipv6_addr *addr, size_t addrn, void *data, uint16_t sz) {
	struct addr_pair aq = *ap;

	if (get_unicast(addr, addrn, &aq.local)) {
		return -1;
	}

	struct eth_buffer *eb = next_ip_buffer(ei, &aq, IP_ICMP6, sz);
	if (!eb) {
		return -1;
	}

	struct icmp6_header *icmp = (struct icmp6_header*) (eb->data + SES_OFF);
	
	char *from = (char*) data + sizeof(struct icmp6_header);
	char *to = (char*) icmp + sizeof(struct icmp6_header);
	memcpy(to, from, sz - sizeof(struct icmp6_header));

	icmp->type = ICMP6_ECHO_RESPONSE;
	icmp->code = 0;
	fill_icmp6_checksum(icmp, sz - sizeof(struct icmp6_header));
	eth_sched_tx(ei, eb, NULL);
	return 0;
}

static int do_send_neighbor_advertisement(struct eth_iface *ei, const struct addr_pair *ap) {
	struct eth_buffer *eb = next_ip_buffer(ei, ap, IP_ICMP6, sizeof(struct icmp6_header) + sizeof(struct neighbor_advertisement));
	if (!eb) {
		return -1;
	}

	struct icmp6_header *icmp = (struct icmp6_header*) (eb->data + SES_OFF);
	struct neighbor_advertisement *neigh = (struct neighbor_advertisement*) (icmp + 1);

	icmp->type = ICMP6_NEIGHBOR_ADVERTISEMENT;
	icmp->code = 0;
	write_big_32(neigh->flags, NDP_OVERRIDE | NDP_SOLICITED);
	write_big_64(&neigh->address[0], ap->local.high);
	write_big_64(&neigh->address[8], ap->local.low);
	neigh->target_len = NDP_OPT_TARGET_LEN;
	neigh->target_type = NDP_OPT_TARGET_TYPE;
	write_big_48(neigh->target_mac, ei->local_mac);

	fill_icmp6_checksum(icmp, sizeof(*neigh));
	eth_sched_tx(ei, eb, NULL);
	return 0;
}

#define NDP_MCAST_HIGH UINT64_C(0xFF02000000000000)
#define NDP_MCAST_LOW  UINT64_C(0x00000001FF000000)
#define NDP_MCAST_MASK UINT64_C(0x0000000000FFFFFF)

int process_neighbor_solicitation(struct eth_iface *ei, const struct addr_pair *ap, const struct ipv6_addr *addr, size_t addrn) {
	struct addr_pair aq = *ap;

	if (aq.local.high == NDP_MCAST_HIGH && (aq.local.low & ~NDP_MCAST_MASK) == NDP_MCAST_LOW) {
		for (size_t i = 0; i < addrn; i++) {
			if ((ap->local.low & NDP_MCAST_MASK) == addr[i].low) {
				aq.local = addr[i];
				return do_send_neighbor_advertisement(ei, &aq);
			}
		}

	} else if (!get_unicast(addr, addrn, &aq.local)) {
		return do_send_neighbor_advertisement(ei, &aq);
	}

	return -1;
}

int send_reset(struct eth_iface *ei, const struct addr_pair *ap) {
	struct eth_buffer *eb = next_tcp_buffer(ei, ap, 0);
	if (!eb) {
		return -1;
	}
    struct tcp_header *tcp = (struct tcp_header*) (eb->data + SES_OFF);
    tcp->flags = TCP_RST;
	fill_tcp_checksum(tcp, 0);
	eth_sched_tx(ei, eb, NULL);
	return 0;
}

static void free_connection(struct tcp_conn *c) {
	c->ei = NULL;
}

int abort_connection(struct tcp_conn *c) {
	free_connection(c);
    return send_reset(c->ei, &c->ap);
}

int send_tcp(struct tcp_conn *c) {
	for (;;) {
		if (c->fin_confirmed && c->fin_received) {
			free_connection(c);
			return 0;
		}

		uint8_t flags = 0;
        int32_t tosend = (int32_t) (c->ack_received + c->send_window - c->seq_sent);

        if (tosend < 0) {
			// the remote closed up their window after we sent some data
            tosend = 0;
        }
        if (tosend > TCP_MAX_MTU) {
			// only one packet at a time
            tosend = TCP_MAX_MTU;
        }
		if (tosend >= c->avail_out) {
			// we've reached the end of the output stream
			tosend = c->avail_out;
			flags |= TCP_FIN;
		}

        if (!flags && !tosend && c->ack_sent == c->seq_received) {
            return 0;
        }

        struct eth_buffer *eb = next_tcp_buffer(c->ei, &c->ap, tosend);
		if (!eb) {
			// if the buffer is full this is picked up on a timeout or tx flush (for sending data)
			// if last_ack_sent != ack_to_send then this is treated as if we never got the ack
			// in the first place. the remote will then resend the data message
			// to deal with these cases, we don't update the connection state above as we may not be able
			// to send out the packet
			return -1;
		}

        struct tcp_header *tcp = (struct tcp_header*) (eb->data + SES_OFF);
        char *msg = (char*) (tcp + 1);

        memcpy(msg, c->next_out, tosend);

		tcp->flags = flags | TCP_ACK;
		write_big_32(tcp->seq, c->seq_sent);
        write_big_32(tcp->ack, c->seq_received);
		fill_tcp_checksum(tcp, tosend);
		eth_sched_tx(c->ei, eb, c);

		// we've successfully sent out the packet, we can now update the connection state to match
        c->seq_sent += tosend;
        c->ack_sent = c->seq_received;
		if (flags & TCP_FIN) {
			c->fin_sent = true;
		}
    }
}

static int copy_receive_data(struct tcp_conn *c, void *data, uint16_t sz) {
	struct tcp_header *tcp = (struct tcp_header*) data;
	uint16_t off = (tcp->offset_reserved >> TCP_OFFSET_SHIFT) * 4;
	char *msg = (char*) tcp + off;
	uint16_t mlen = sz - off;

	if (mlen > c->avail_in) {
		// also handles case where remote sends data after FIN
		// as c->avail_in is set to 0
		return abort_connection(c);
	}

	memcpy(c->next_in, msg, mlen);
	c->next_in += mlen;
	c->avail_in -= mlen;

	if (tcp->flags & TCP_FIN) {
		c->fin_received = true;
		c->avail_in = 0;
	}

	return 0;
}

static int resend_window(struct tcp_conn *c, uint32_t tick) {
	// set the last ack to something else to force out an ack
	c->ack_sent--;
	c->seq_sent = c->ack_received;
	c->fin_sent = false;
	c->dupacks = 0;
	c->last_rx_tick = tick;
	return send_tcp(c);
}

int process_tcp(struct tcp_conn *c, uint32_t tick, void *data, uint16_t sz) {
	struct tcp_header *tcp = (struct tcp_header*) data;

	if (tcp->flags & TCP_RST) {
		free_connection(c);
		return -1;
	}

	c->last_rx_tick = tick;
	
	if (tcp->flags & TCP_ACK) {
		uint32_t ack = read_big_32(tcp->ack);
		int32_t amount = (int32_t) (ack - c->ack_received);
		int32_t unacked = (int32_t) (c->seq_sent - c->ack_received);

		if (amount > unacked) {
			// we've received an ack for data beyond what we've sent
			return abort_connection(c);
		}
		if (amount < 0) {
			// we've received an older ack. this may be due to out of order delivery
			return 0;
		}

		if (amount != 0) {
			c->dupacks = 0;
		} else if (++c->dupacks == 2) {
			// we've had three (original + two dups) acks in a row
			// the other side has likely dropped a packet
			// so perform dupack based retransmission
			return resend_window(c, tick);
		}

		c->avail_out -= amount;
		c->next_out += amount;
		c->ack_received = ack;

		if (c->fin_sent && c->ack_received == c->seq_sent) {
			c->fin_confirmed = true;
		}
	}

	uint32_t seq = read_big_32(tcp->seq);

	if (seq != c->ack_sent) {
		// we don't process out of order data
		return 0;
	}

	return copy_receive_data(c, data, sz);
}

int process_tick(struct tcp_conn *c, uint32_t tick) {
	uint32_t t = tick - c->last_rx_tick;
	if (t < RETRY_TICKS) {
		return 0;
	} else if (t < DROP_TICKS) {
		// timeout based restransmission
		return resend_window(c, tick);
	} else {
		// no data timeout
		return abort_connection(c);
	}
}


#define ADDR_PAIR_PACKED (16+16+2+2)
static_assert(offsetof(struct addr_pair, type) - offsetof(struct addr_pair, local) == ADDR_PAIR_PACKED, "padding needs to be at the end for sha1 to work");

static uint32_t generate_syn_cookie(struct addr_pair *ap, uint32_t minute) {
    // standard for cookies is for the top 5 bits to indicate the minute
    // the cookie was generated in
    uint32_t t = minute & 0x1F;
    uint32_t m = 0;

    // store this in here temporarily so we can make a single sha1 call
    uint8_t type = ap->type;
    ap->type = (uint8_t) t;

	// poor man's hash of time, addresses and ports
	uint32_t sum = 0x12345678;
	sum += minute;

	uint32_t *p = (uint32_t*) &ap->local;
	for (int i = 0; i < ADDR_PAIR_PACKED / 4; i++) {
		sum += p[i];
	}

    ap->type = type;

	sum &= 0xFFFFFF;
	sum |= (m << 24);
	sum |= (t << 27);
	return sum;
}


int accept_syn_cookie(struct tcp_conn *c, struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz) {
    struct tcp_header *tcp = (struct tcp_header*) data;

	if (tcp->flags & TCP_RST) {
		return -1;
	}

    uint32_t now_minute = tick >> TICK_MINUTE_SHIFT;
    uint32_t ack = read_big_32(tcp->ack);
    uint32_t syn_minute = (ack >> 27) & 0x1F;
    uint32_t cookie = generate_syn_cookie((struct addr_pair*) ap, syn_minute);

    // the tick check catches if the syn was more than 64 seconds ago
    // or is from the future because it's unsigned
    if (!(tcp->flags & TCP_ACK) || (tcp->flags & TCP_FIN) || now_minute - syn_minute > 1 || cookie != ack) {
		send_reset(ei, ap);
        return -1;
    }

	uint32_t seq = read_big_32(tcp->seq);

	c->ei = ei;
    c->ap = *ap;
	c->fin_sent = false;
	c->fin_confirmed = false;
	c->dupacks = 0;
    c->ack_received = ack;
    c->seq_sent = ack;
    c->send_window = read_big_16(tcp->window);
	c->fin_received = false;
	c->seq_received = seq;
	c->ack_sent = seq - 1; // make sure we force out an ACK
	c->last_rx_tick = tick;

	return copy_receive_data(c, data, sz);
}

int send_syn_cookie(struct eth_iface *ei, const struct addr_pair *ap, uint32_t tick, void *data, uint16_t sz) {
	struct tcp_header *tcpin = (struct tcp_header*) data;

    uint32_t cookie = generate_syn_cookie((struct addr_pair *) ap, tick >> TICK_MINUTE_SHIFT);

	struct eth_buffer *eb = next_tcp_buffer(ei, ap, 0);
	if (!eb) {
		return -1;
	}
    struct tcp_header *tcp = (struct tcp_header*) (eb->data + SES_OFF);

	tcp->flags = TCP_ACK | TCP_SYN;
	write_big_32(tcp->seq, cookie);
	memcpy(tcp->ack, tcpin->seq, sizeof(tcp->ack));
	fill_tcp_checksum(tcp, 0);
	eth_sched_tx(ei, eb, NULL);
	return 0;
}

#include "hw_mac.h"
#include "tcp.h"
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof((ar)[0]))

#define NUM_CONNECTIONS 8
#define RECEIVE_BUFSZ 512
static struct tcp_conn g_conns[NUM_CONNECTIONS];

struct tcp_conn *lookup_conn(struct eth_iface *ei, const struct addr_pair *ap) {
	for (int i = 0; i < NUM_CONNECTIONS; i++) {
		struct tcp_conn *c = &g_conns[i];
		if (c->ei == ei && (ap == NULL || addr_equal(&c->ap, ap))) {
			return c;
		}
	}
	return NULL;
}

char tcp_cookie_key[32] = "1234567890123456789012";

int main(int argc, char *argv[]) {
	// put this on the stack so we don't clear it on init
	char recvbuf[RECEIVE_BUFSZ * NUM_CONNECTIONS];

	uint64_t macs[] = {
		UINT64_C(0x123456789ABC),
		UINT64_C(0x3333ff000002), // NDP multicast address ff02::01ff:0:2
		UINT64_C(0x333300000001), // all nodes multicast ff02::1
	};
	struct ipv6_addr addrs[] = {
		{UINT64_C(0xFE80000000000000), UINT64_C(0x0000000000000002)},
		{UINT64_C(0xFF02000000000000), UINT64_C(0x0000000000000001)}, // all nodes ff02::1
	};

	struct eth_iface ei;
	if (eth_init(&ei, argc > 1 ? argv[1] : "", macs, ARRAY_SIZE(macs))) {
		return 2;
	}

	uint32_t tick = 0;

	for (;;) {
		int events = hw_emac_poll(&ei, &tick);

		if (events & HW_EMAC_SENT) {
			struct eth_buffer *eb;

			while ((eb = eth_tx_finished(&ei)) != NULL) {
				struct tcp_conn *c = (struct tcp_conn*) eb->udata;
				if (c && c->ei) {
					send_tcp(c);
				}
			}
		}


		if (events & HW_EMAC_RECEIVED) {
			struct eth_buffer *eb;
			struct addr_pair ap;
			while ((eb = eth_rx_finished(&ei)) != NULL) {
				if ((eb->flags & ETH_FULL_PACKET) != ETH_FULL_PACKET) {
					continue;
				}

				void *data = eb->data;
				uint16_t len = eb->packet_length;
				if (process_ethernet(&ei, &data, &len, &ap)) {
					continue;
				}

				switch (ap.type) {
				case IP_NDP_SOLICIT:
					process_neighbor_solicitation(&ei, &ap, addrs, ARRAY_SIZE(addrs));
					break;
				case IP_PING:
					process_echo_request(&ei, &ap, addrs, ARRAY_SIZE(addrs), data, len);
					break;
				case IP_TCP_NEW:
					if (ap.local_port == 80) {
						send_syn_cookie(&ei, &ap, tick, data, len);
					} else {
						send_reset(&ei, &ap);
					}
					break;
				case IP_TCP:
				{
					struct tcp_conn *c = lookup_conn(&ei, &ap);
					int err;
					if (c) {
						err = process_tcp(c, tick, data, len);
					} else if ((c = lookup_conn(NULL, NULL)) != NULL) {
						c->next_in = recvbuf + (RECEIVE_BUFSZ * (c - g_conns));
						c->avail_in = RECEIVE_BUFSZ;
						c->next_out = "hello world\n";
						c->avail_out = strlen("hello world\n");
						err = accept_syn_cookie(c, &ei, &ap, tick, data, len);
					} else {
						send_reset(&ei, &ap);
						break;
					}

					if (!err) {
						char *begin = recvbuf + (RECEIVE_BUFSZ * (c - g_conns));
						fprintf(stderr, "have '%.*s'\n", c->avail_in - RECEIVE_BUFSZ, begin);
						send_tcp(c);
					}
				}
				break;
				}
			}
		}

		if (events & HW_EMAC_TICKED) {
			for (int i = 0; i < NUM_CONNECTIONS; i++) {
				struct tcp_conn *c = &g_conns[i];
				if (c->ei) {
					process_tick(c, tick);
				}
			}
		}
	}

}


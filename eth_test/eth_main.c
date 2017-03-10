#include "hw_mac.h"
#include "tcp.h"
#include <stdio.h>

#define NUM_CONNECTIONS 32
struct tcp_conn g_conns[NUM_CONNECTIONS];
struct eth_iface g_iface;

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
	if (argc < 2) {
		fprintf(stderr, "usage: ./eth_main netdev\n");
		return 2;
	}
	if (eth_init(&g_iface, argv[1], 0x34e6d72b4e3c)) {
		return 2;
	}

	uint32_t tick = 0;

	for (;;) {
		int events = hw_emac_poll(&g_iface, &tick);

		if (events & HW_EMAC_TICKED) {
			for (int i = 0; i < NUM_CONNECTIONS; i++) {
				struct tcp_conn *c = &g_conns[i];
				if (c->ei) {
					process_tick(c, tick);
				}
			}
		}

		if (events & HW_EMAC_RECEIVED) {
			struct eth_buffer *eb;
			struct addr_pair ap;
			while ((eb = eth_rx_finished(&g_iface)) != NULL) {
				if ((eb->flags & ETH_FULL_PACKET) != ETH_FULL_PACKET) {
					continue;
				}

				void *data = eb->data;
				uint16_t len = eb->packet_length;
				if (process_ethernet(&g_iface, &data, &len, &ap)) {
					continue;
				}

				switch (ap.type) {
				case IP_TCP_NEW:
					if (ap.local_port == 80) {
						send_syn_cookie(&g_iface, &ap, tick, data, len);
					} else {
						send_reset(&g_iface, &ap);
					}
					break;
				case IP_TCP:
				{
					struct tcp_conn *c = lookup_conn(&g_iface, &ap);
					if (c) {
						process_tcp(c, tick, data, len);
					} else if ((c = lookup_conn(NULL, NULL)) != NULL) {
						accept_syn_cookie(c, &g_iface, &ap, tick, data, len);
					} else {
						send_reset(&g_iface, &ap);
					}
				}
				break;
				}
			}
		}

		if (events & HW_EMAC_SENT) {
			struct eth_buffer *eb;

			while ((eb = eth_tx_finished(&g_iface)) != NULL) {
				struct tcp_conn *c = (struct tcp_conn*) eb->udata;
				if (c->ei) {
					send_tcp(c);
				}
			}
		}
	}

}


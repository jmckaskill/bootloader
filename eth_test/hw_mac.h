#pragma once

#include "eth.h"

int hw_emac_open(struct eth_iface *ei, const char *port, const uint64_t *mac, size_t macn);
void hw_emac_close(struct eth_iface *ei);

#define HW_EMAC_PENDING 1
#define HW_EMAC_ERROR -1
#define HW_EMAC_SUCCESS 0
int hw_emac_send(struct eth_iface *ei, struct eth_buffer *eb);
void hw_emac_recv(struct eth_iface *ei, struct eth_buffer *eb);

#define HW_EMAC_TICKED	1
#define HW_EMAC_RECEIVED	2
#define HW_EMAC_SENT 4
int hw_emac_poll(struct eth_iface *ei, uint32_t *ptick);

void reset_data_cache(volatile void *p, size_t sz);


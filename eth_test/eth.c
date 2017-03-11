#include "eth.h"
#include "hw_mac.h"
#include "endian.h"
#include "crc.h"
#include <stddef.h>
#include <assert.h>

// eth_init initializes a new ethernet interface
int eth_init(struct eth_iface *ei, const char *port, const uint64_t *mac, size_t macn) {
	ei->hw_udata = NULL;
	ei->local_mac = mac[0];
	ei->txtail = -1;
	ei->txhead = -1;
	ei->rxnext = ei->rxq;

	struct eth_buffer *prev = &ei->rxq[ETH_RX_QUEUE_SIZE - 1];

	for (int i = 0; i < ETH_RX_QUEUE_SIZE; i++) {
		struct eth_buffer *b = &ei->rxq[i];
		b->next = (i == (ETH_RX_QUEUE_SIZE-1) ? NULL : (b + 1));
		b->data = b->buf;
		b->offset = 0;
		b->length = ETH_BUFFER_SIZE;
		b->flags = ETH_OWNED_BY_MAC | ETH_FULL_PACKET;
		b->prev = prev;
		prev = b;
	}

	if (hw_emac_open(ei, port, mac, macn)) {
		return -1;
	}

	hw_emac_recv(ei, ei->rxq);
	return 0;
}

void eth_destroy(struct eth_iface *ei) {
	hw_emac_close(ei);
}

// eth_tx_finished should be called in a loop after a tx interrupt is generated
// on each call it will pop the next finished buffer off the queue
// or return NULL when there is no more finished buffers
// the returned buffer is only valid until the next eth_tx_finished or eth_next_tx call
struct eth_buffer *eth_tx_finished(struct eth_iface *ei) {
	if (ei->txhead < 0) {
		return NULL;
	}

	struct eth_buffer *eb = &ei->txq[ei->txhead];
	reset_data_cache(&eb->flags, sizeof(eb->flags));
	if (eb->flags & ETH_OWNED_BY_MAC) {
		return NULL;
	}

	if (ei->txhead == ei->txtail) {
		ei->txhead = ei->txtail = -1;
	} else {
		ei->txhead = (ei->txhead + 1) % ETH_TX_QUEUE_SIZE;
	}

	return eb;
}

// eth_rx_finished should be called in a loop after a rx interrupt is generated
// on each call it will pop the next received buffer off the queue
// or return NULL when there are no more received packets
// the returned buffer is only valid until the next call to eth_rx_finished
struct eth_buffer *eth_rx_finished(struct eth_iface *ei) {
	struct eth_buffer *eb = ei->rxnext;
	reset_data_cache(&eb->flags, sizeof(struct eth_buffer) - offsetof(struct eth_buffer, flags));
	if (eb->flags & ETH_OWNED_BY_MAC) {
		return NULL;
	}

	ei->rxnext = eb->next;

	struct eth_buffer *last = eb->prev;
	last->prev->next = last;
	last->next = NULL;
	last->flags = ETH_OWNED_BY_MAC | ETH_FULL_PACKET;
	last->packet_length = 0;

	hw_emac_recv(ei, ei->rxnext);

	return eb;
}

// eth_next_tx returns the next send buffer
// it will return NULL if the queue is full
struct eth_buffer *eth_next_tx(struct eth_iface *ei, uint16_t len) {
	int idx = (ei->txtail + 1) % ETH_TX_QUEUE_SIZE;
	if (idx == ei->txhead) {
		return NULL;
	}

	struct eth_buffer *b = &ei->txq[idx];
	b->next = NULL;
	b->data = b->buf;
	b->offset = 0;
	b->length = len;
	b->packet_length = len;
	b->flags = ETH_START_OF_PACKET | ETH_END_OF_PACKET | ETH_OWNED_BY_MAC;
	return b;
}

// eth_sched_tx schedules the next buffer to be sent
// the supplied buffer must have been just retrieved from eth_next_tx
void eth_sched_tx(struct eth_iface *ei, struct eth_buffer *eb, void *udata) {
#if 0
	uint8_t *pfcs = eb->data + eb->length - 4;
	uint32_t fcs = crc32(eb->data, eb->length - 4);
	write_little_32(pfcs, fcs);
#endif

	assert(eb == &ei->txq[(ei->txtail + 1) % ETH_TX_QUEUE_SIZE]);
	eb->udata = udata;
	if (ei->txtail >= 0) {
		ei->txq[ei->txtail].next = eb;
		ei->txtail = (ei->txtail + 1) % ETH_TX_QUEUE_SIZE;
		return;
	}

	if (hw_emac_send(ei, eb) == HW_EMAC_PENDING) {
		ei->txhead = ei->txtail = 0;
	} else {
		ei->txhead = ei->txtail = -1;
	}
}

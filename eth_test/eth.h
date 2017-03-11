#pragma  once
#include <stddef.h>
#include <stdint.h>

#define ETH_BUFFER_SIZE 1536

#pragma pack(push)
#pragma pack(1)
// eth_buffer is the buffer structure for interacting with the CPU MAC
struct eth_buffer {
	struct eth_buffer *next;
	char *data;
	uint16_t offset;
	uint16_t length;
	volatile uint16_t flags;
	volatile uint16_t packet_length;

	// not used by the MAC driver
	struct eth_buffer *prev;
	void *udata;
	char buf[ETH_BUFFER_SIZE];
};
#pragma pack(pop)

#define ETH_RX_QUEUE_SIZE 8
#define ETH_TX_QUEUE_SIZE 8

struct eth_iface {
	void *hw_udata;
	uint64_t local_mac;
	int txhead, txtail;
	struct eth_buffer *rxnext;
	struct eth_buffer rxq[ETH_RX_QUEUE_SIZE];
	struct eth_buffer txq[ETH_TX_QUEUE_SIZE];
};

// Common flags
#define ETH_START_OF_PACKET		0x8000
#define ETH_END_OF_PACKET		0x4000
#define ETH_FULL_PACKET			0xC000
#define ETH_OWNED_BY_MAC		0x2000
#define ETH_END_OF_QUEUE		0x1000
#define ETH_TEARDOWN_COMPLETE	0x0800
#define ETH_INCLUDES_CRC		0x0400

// TX flags
#define ETH_TO_FIRST_PORT		0x0001
#define ETH_TO_SECOND_PORT		0x0011

// RX flags
#define ETH_CONTROL_FRAME		0x0080
#define ETH_RX_ERROR			0x0070
#define ETH_INCLUDES_VLAN		0x0008
#define ETH_FROM_SECOND_PORT	0x0001

int eth_init(struct eth_iface *ei, const char *port, const uint64_t *mac, size_t macn);
void eth_destroy(struct eth_iface *ei);

struct eth_buffer *eth_tx_finished(struct eth_iface *ei);
struct eth_buffer *eth_rx_finished(struct eth_iface *ei);

struct eth_buffer *eth_next_tx(struct eth_iface *ei, uint16_t len);
void eth_sched_tx(struct eth_iface *ei, struct eth_buffer *eb, void *udata);



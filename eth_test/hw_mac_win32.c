#include "hw_mac.h"
#include <stdio.h>
#if 0

#include "../npcap-sdk/Include/Packet32.h"

#include <ntddndis.h>

int hw_emac_open(struct eth_iface *ei, const char *port) {
	LPADAPTER iface = PacketOpenAdapter((char*) port);
	if (!iface || iface->hFile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "failed to open port %s\n", port);
		fprintf(stderr, "valid port names are\n");
		char buf[8192];
		int buflen = sizeof(buf);
		if (PacketGetAdapterNames(buf, &buflen)) {
			fprintf(stderr, "valid port names are:\n");
			char *p = buf;
			size_t sz;
			while ((sz = strlen(p)) != 0) {
				fprintf(stderr, " %s\n", p);
				p += sz + 1;
			}
		}
		system("getmac /V");
		return -1;
	}

	if (!PacketSetHwFilter(iface, NDIS_PACKET_TYPE_PROMISCUOUS)) {
		goto err;
	}

	if (!PacketSetBuff(iface, 512000)) {
		goto err;
	}

	// the read timeout gives us our 10 Hz tick
	if (!PacketSetReadTimeout(iface, 100)) {
		goto err;
	}

	ei->hw_udata = iface;
	return 0;

err:
	PacketCloseAdapter(iface);
	return -1;
}

void hw_emac_close(struct eth_iface *ei) {
	PacketCloseAdapter((LPADAPTER) ei->hw_udata);
	ei->hw_udata = NULL;
}

int hw_emac_send(struct eth_iface *ei, struct eth_buffer *eb) {
	LPADAPTER iface = (LPADAPTER) ei->hw_udata;
	LPPACKET packet = PacketAllocatePacket();
	PacketInitPacket(packet, eb->data, eb->packet_length);
	BOOL ok = PacketSetNumWrites(iface, 1);
	ok = ok && PacketSendPacket(iface, packet, TRUE);
	eb->flags &= ~ETH_OWNED_BY_MAC;
	PacketFreePacket(packet);
	return ok ? HW_EMAC_PENDING : HW_EMAC_ERROR;
}

static struct eth_buffer *g_next_poll;

int hw_emac_poll(struct eth_iface *ei, uint32_t *ptick) {
	LPPACKET packet = PacketAllocatePacket();
	struct eth_buffer *eb = g_next_poll;
	PacketInitPacket(packet, eb->data, eb->length);

	int ret = 0;
	if (PacketReceivePacket((LPADAPTER) ei->hw_udata, packet, TRUE)) {
		ret |= HW_EMAC_RECEIVED;
		eb->packet_length = (uint16_t) packet->ulBytesReceived;
		eb->flags &= ~ETH_OWNED_BY_MAC;
		if (eb->packet_length > 22) {
			memmove(eb->data, eb->data + 20, eb->packet_length - 22);
			eb->packet_length -= 22;
		}
	}

	uint32_t tick = (uint32_t) (GetTickCount64() / 100);
	if (tick != *ptick) {
		ret |= HW_EMAC_TICKED;
		*ptick = tick;
	}

	PacketFreePacket(packet);
	return ret;
}

void hw_emac_recv(struct eth_iface *ei, struct eth_buffer *eb) {
	g_next_poll = eb;
}

void reset_data_cache(volatile void *p, size_t sz) {
	(void) p;
	(void) sz;
}

#endif

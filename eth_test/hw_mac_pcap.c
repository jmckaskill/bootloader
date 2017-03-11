#include "hw_mac.h"
#include "endian.h"
#include <inttypes.h>
#include <pcap.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/poll.h>
#endif

#ifdef NDEBUG
#define log(...) do {} while(0)
#else
#define log(...) fprintf(stderr, __VA_ARGS__)
#endif

struct mac_pcap {
	pcap_t *p;
	struct eth_buffer *recv, *send;
	int received;
	size_t macn;
	uint64_t mac[1];
};

int hw_emac_open(struct eth_iface *ei, const char *port, const uint64_t *mac, size_t macn) {
    bool promiscous = true;
    char errbuf[512];
    pcap_t *iface = pcap_open_live(port, ETH_BUFFER_SIZE, promiscous, 0, errbuf);
    if (!iface) {
        log("failed to open device '%s'\n", errbuf);

		pcap_if_t *alldevs;
		pcap_if_t *d;
		int i = 0;
		char errbuf[PCAP_ERRBUF_SIZE];

		/* Retrieve the device list from the local machine */
		if (pcap_findalldevs(&alldevs, errbuf) == -1) {
			log("Error in pcap_findalldevs: %s\n", errbuf);
			return -1;
		}

		log("available devices:\n");

		/* Print the list */
		for (d = alldevs; d != NULL; d = d->next) {
			log("%d. %s", ++i, d->name);
			if (d->description)
				log(" (%s)\n", d->description);
			else
				log(" (No description available)\n");
		}

		if (i == 0) {
			log("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return -1;
		}

		pcap_freealldevs(alldevs);
        return -1;
    }

    if (pcap_set_datalink(iface, DLT_EN10MB)) {
        log("set datalink\n");
    }
    if (pcap_setnonblock(iface, 1, errbuf)) {
        log("nonblock %s\n", errbuf);
    }

	struct mac_pcap *m = (struct mac_pcap *) calloc(1, sizeof(struct mac_pcap) + macn * sizeof(*mac));
	m->p = iface;
	m->macn = macn;
	for (size_t i = 0; i < macn; i++) {
		m->mac[i] = mac[i];
	}

    ei->hw_udata = m;
    return 0;
}

void hw_emac_close(struct eth_iface *ei) {
	struct mac_pcap *m = (struct mac_pcap*) ei->hw_udata;
    pcap_close(m->p);
	free(m);
    ei->hw_udata = NULL;
}

static uint32_t get_tick() {
#ifdef WIN32
	return (uint32_t) (GetTickCount64() / 100);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t) (tv.tv_sec * 10) + (uint32_t) (tv.tv_usec / 1000);
#endif
}

static bool should_filter(struct mac_pcap *m, uint64_t mac) {
	for (size_t i = 0; i < m->macn; i++) {
		if (m->mac[i] == mac) {
			return false;
		}
	}
	return true;
}

static void dispatch(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
	struct mac_pcap *m = (struct mac_pcap*) user; 
	if (h->caplen < 6) {
		return;
	}
	
	uint64_t dstmac = read_big_48(bytes);
	//log("MAC %"PRIx64"\n", dstmac);
	if (should_filter(m, dstmac)) {
		return;
	}

	struct eth_buffer *eb = m->recv;
    assert(h->caplen <= eb->length);
    memcpy(eb->data, bytes, h->caplen);
    eb->packet_length = h->caplen;
    eb->flags &= ~ETH_OWNED_BY_MAC;
    m->recv = m->recv->next;
	m->received++;
}

static int collect_packets(struct mac_pcap *m) {
	int cnt = 0;
	for (struct eth_buffer *eb = m->recv; eb != NULL; eb = eb->next) {
		cnt++;
	}

	m->received = 0;
	pcap_dispatch(m->p, cnt, &dispatch, (u_char*) m);
	return m->received;
}

int hw_emac_poll(struct eth_iface *ei, uint32_t *ptick) {
	struct mac_pcap *m = (struct mac_pcap*) ei->hw_udata;

    int ret = 0;

    if (m->send) {
        while (m->send) {
            m->send->flags &= ~ETH_OWNED_BY_MAC;
            m->send = m->send->next;
        }
		ret |= HW_EMAC_SENT;
    }

	if (collect_packets(m)) {
		ret |= HW_EMAC_RECEIVED;
	}

	if (!ret) {
#ifdef WIN32
		HANDLE ev = pcap_getevent(m->p);
		WaitForSingleObject(ev, 100);
#else
		int fd = pcap_get_selectable_fd(m->p);
		struct pollfd pfd = {fd, POLLIN, 0};
		poll(&pfd, 1, 100);
#endif
		if (collect_packets(m)) {
			ret |= HW_EMAC_RECEIVED;
		}
	}

	uint32_t tick = get_tick();
	if (tick != *ptick) {
		ret |= HW_EMAC_TICKED;
	}

    return ret;
}

void hw_emac_recv(struct eth_iface *ei, struct eth_buffer *eb) {
	struct mac_pcap *m = (struct mac_pcap*) ei->hw_udata;
	m->recv = eb;
}

int hw_emac_send(struct eth_iface *ei, struct eth_buffer *eb) {
	struct mac_pcap *m = (struct mac_pcap*) ei->hw_udata;
	if (pcap_sendpacket(m->p, eb->data, eb->length) != 0) {
		log("Error sending the packet: %s\n", pcap_geterr(m->p));
		return HW_EMAC_ERROR;
	}
    if (!m->send) {
        m->send = eb;
    }
    return HW_EMAC_PENDING;
}

void reset_data_cache(volatile void *p, size_t sz) {
	(void) p;
	(void) sz;
}

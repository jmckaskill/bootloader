#include "hw_mac.h"
#include <pcap.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <sys/poll.h>

int hw_emac_open(struct eth_iface *ei, const char *port) {
    bool promiscous = true;
    char errbuf[512];
    pcap_t *iface = pcap_open_live(port, ETH_BUFFER_SIZE, promiscous, 0, errbuf);
    if (!iface) {
        fprintf(stderr, "failed to open %s\n", errbuf);
        return -1;
    }

    if (pcap_set_datalink(iface, DLT_EN10MB)) {
        fprintf(stderr, "set datalink\n");
    }
    if (pcap_setnonblock(iface, 1, errbuf)) {
        fprintf(stderr, "nonblock %s\n", errbuf);
    }

    ei->hw_udata = iface;
    return 0;
}

void hw_emac_close(struct eth_iface *ei) {
    pcap_close((pcap_t*) ei->hw_udata);
    ei->hw_udata = NULL;
}

static uint32_t get_tick() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t) (tv.tv_sec * 10) + (uint32_t) (tv.tv_usec / 1000);
}

static struct eth_buffer *g_sent, *g_recv;

static void dispatch(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
    struct eth_buffer *eb = g_recv;
    assert(h->caplen <= eb->length);
    memcpy(eb->data, bytes, h->caplen);
    eb->packet_length = h->caplen;
    eb->flags &= ~ETH_OWNED_BY_MAC;
    g_recv = g_recv->next;
}

int hw_emac_poll(struct eth_iface *ei, uint32_t *ptick) {
    if (g_sent) {
        while (g_sent) {
            g_sent->flags &= ~ETH_OWNED_BY_MAC;
            g_sent = g_sent->next;
        }
        return HW_EMAC_SENT;
    }

    pcap_t *iface = (pcap_t*) ei->hw_udata;
    int fd = pcap_get_selectable_fd(iface);
    struct pollfd pfd = {fd, POLLIN, 0};
    int r = poll(&pfd, 1, 100);

    int ret = 0;
    uint32_t tick = get_tick();
    if (tick != *ptick) {
        ret |= HW_EMAC_TICKED;
    }
    if (r > 0) {
        int cnt = 0;
        for (struct eth_buffer *eb = g_recv; eb != NULL; eb = eb->next) {
            cnt++;
        }

        if (pcap_dispatch(iface, cnt, &dispatch, NULL)) {
            ret |= HW_EMAC_RECEIVED;
        }
    }
    return ret;
}

void hw_emac_recv(struct eth_iface *ei, struct eth_buffer *eb) {
    g_recv = eb;
}

int hw_emac_send(struct eth_iface *ei, struct eth_buffer *eb) {
    pcap_t *iface = (pcap_t*) ei->hw_udata;
    pcap_inject(iface, eb->data, eb->length);
    if (!g_sent) {
        g_sent = eb;
    }
    return HW_EMAC_PENDING;
}

void reset_data_cache(volatile void *p, size_t sz) {
	(void) p;
	(void) sz;
}

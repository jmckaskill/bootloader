#include "tcp.h"

static struct tcp_conn g_conns[32];

struct tcp_conn *lookup_conn(const struct addr_pair *ap) {
    for (int i = 0; i < sizeof(g_conns) / sizeof(g_conns[0]); i++) {
        struct tcp_conn *c = &g_conns[i];
        if (c->connected && addr_equal(&c->ap, ap)) {
            return c;
        }
    }
    return NULL;
}

struct tcp_conn *free_conn() {
    for (int i = 0; i < sizeof(g_conns) / sizeof(g_conns[0]); i++) {
        struct tcp_conn *c = &g_conns[i];
        if (!c->connected) {
            return c;
        }
    }
    return NULL;
}

void on_ethernet(const void *data, int sz) {
    struct addr_pair ap;
    if (process_ethernet(&data, &sz, &ap)) {
        return;
    }

    switch (ap.type) {
    case IP_TCP_NEW:
        if (ap.local_port == 80) {
            send_syn_cookie(&ap);
        } else {
            send_reset(&ap);
        }
        break;
    case IP_TCP:
        {
            struct tcp_conn *c = lookup_conn(&ap);
            if (c) {
                process_tcp(c, data, sz);
            } else if ((c = free_conn()) != NULL) {
                accept_syn_cookie()
            } else {
                send_reset(&ap);
            }
        }
        break;
    }
}

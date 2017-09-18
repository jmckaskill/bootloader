#include "../eth.h"
#include "unit_test.h"
#include <string.h>
#include <assert.h>

void init_ethernet(uint8_t *mac) {
    memcpy(mac, "\x01\x02\x03\x04\x05\x06", 6);
}

void check_ethernet_link() {}

#define MAX_TX_ETH 16
struct test_eth test_tx[MAX_TX_ETH];
size_t test_tx_len;

void *new_eth_frame() {
    if (test_tx_len == MAX_TX_ETH) {
        return NULL;
    }

    return test_tx[test_tx_len].data;
}


void send_eth_frame(void *data, int sz) {
    assert(data == test_tx[test_tx_len].data);
    test_tx[test_tx_len++].sz = sz;
}



#pragma once

struct test_eth {
    int sz;
    uint8_t data[1500];
};

extern struct test_eth test_tx[];
extern size_t text_tx_len;

extern unsigned test_tick;


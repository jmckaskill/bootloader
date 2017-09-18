#pragma once

#include <stdint.h>


void init_ethernet(uint8_t *mac);
void check_ethernet_link();

void *new_eth_frame();
void send_eth_frame(void *data, int sz);


extern void ethernet_up();
extern void ethernet_down();
extern void process_eth_frame(void *data, int sz);


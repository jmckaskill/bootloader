#pragma once

#include <stdint.h>
#include "ip6.h"

void init_ethernet();
void check_ethernet_link();

struct ip6_header *new_eth_frame(const mac_addr_t *dst_mac, const ip6_addr_t *dst_ip, uint8_t type);
void send_eth_frame(int sz);

void process_eth_rx_interrupt();
void process_eth_tx_interrupt();
void process_eth_misc_interrupt();

extern void ethernet_up();
extern void ethernet_down();
extern void process_ip6(struct ip6_header *e, uint8_t type, const void *msg, int sz);

extern mac_addr_t g_my_mac;
extern ip6_addr_t g_my_ip;





#pragma once

void interrupt();

void process_eth_rx_interrupt();
void process_eth_tx_interrupt();
void process_eth_misc_interrupt();

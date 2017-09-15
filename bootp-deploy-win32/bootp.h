#pragma once

#include <stdint.h>

struct bootp;

struct bootp_client {
	uint8_t mac[6];
	char vendor_class[32];
	uint64_t sent;

	// set by the application
	const uint8_t *data; 
	uint64_t size;
};

typedef void (*bootp_cb)(void *udata, struct bootp_client* c);

struct bootp *bootp_start(int interface_id, void *udata, bootp_cb cb);
void bootp_stop(struct bootp *b);



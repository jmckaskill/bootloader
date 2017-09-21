#include "bootp.h"
#include <windows.h>

extern const char g_data[];
extern unsigned long g_datasz;

void bootp_callback(void *udata, struct bootp_client *c) {
	if (!strcmp(c->vendor_class, "AM335x ROM")) {
		c->data = (uint8_t*) g_data;
		c->size = g_datasz;
	}
}

int main() {
	bootp_start(24, NULL, &bootp_callback);
	Sleep(INFINITE);
	return 0;
}

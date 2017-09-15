#include "bootp.h"
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#endif

#define NUM_CLIENTS 32
#define SERVER_ADDRESS_SPLIT 10, 186, 26, 1
#define SUBNET 0x0ABA1A00 // 10.186.26.0
#define SERVER_ADDRESS (SUBNET+1)
#define FIRST_CLIENT (SUBNET+100)
#define NETMASK 0xFFFFFF00 // 255.255.255.0

#define BOOTP_SERVER_PORT 67
#define BOOTP_CLIENT_PORT 68
#define TFTP_PORT 69

struct bootp {
	uint32_t clients_free;
	struct bootp_client clients[NUM_CLIENTS];
	HANDLE stop;
	HANDLE thread;
	bootp_cb cb;
	void *udata;
	int bootp;
	int tftp;
	int interface_id;
};

static inline void put_big_16(uint8_t *u, uint16_t v) {
	u[0] = (uint8_t) (v >> 8);
	u[1] = (uint8_t) v;
}

static inline void put_big_32(uint8_t *u, uint32_t v) {
	u[0] = (uint8_t) (v >> 24);
	u[1] = (uint8_t) (v >> 16);
	u[2] = (uint8_t) (v >> 8);
	u[3] = (uint8_t) (v);
}

static inline uint16_t big_16(const uint8_t *u) {
	return ((uint16_t) u[0] << 8) | ((uint16_t) u[1]);
}

static inline uint32_t big_32(const uint8_t *u) {
	return ((uint32_t) u[0] << 24)
		|  ((uint32_t) u[1] << 16)
		|  ((uint32_t) u[2] << 8)
		|  ((uint32_t) u[3]);
}

static bool is_server_address(const MIB_UNICASTIPADDRESS_ROW* row) {
	return row->Address.si_family == AF_INET && row->Address.Ipv4.sin_addr.s_addr == ntohl(SERVER_ADDRESS);
}

static int setup_server_address(int interface_id) {
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);

	MIB_UNICASTIPADDRESS_TABLE *table;
	if (GetUnicastIpAddressTable(AF_INET, &table) != NO_ERROR) {
		return -1;
	}

	bool have_address = false;

	// remove our server address from other interfaces
	for (size_t i = 0; i < table->NumEntries; i++) {
		const MIB_UNICASTIPADDRESS_ROW *row = &table->Table[i];
		if (is_server_address(row)) {
			if ((int) row->InterfaceIndex == interface_id) {
				have_address = true;
			} else {
				DeleteUnicastIpAddressEntry(row);
			}
		}
	}

	FreeMibTable(table);

	if (have_address) {
		return 0;
	}

	unsigned long nte_context, nte_instance;
	int err = AddIPAddress(ntohl(SERVER_ADDRESS), ntohl(NETMASK), interface_id, &nte_context, &nte_instance);
	if (err != NO_ERROR) {
		return -1;
	}

	return 0;
}

static int create_socket(uint16_t port) {
	int fd = (int) socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		return -1;
	}
	
	long rcv_broadcast = 1;
	if (setsockopt(fd, IPPROTO_IP, IP_RECEIVE_BROADCAST, (char*) &rcv_broadcast, sizeof(rcv_broadcast))) {
		goto err;
	}

	struct sockaddr_in sa = {0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ntohl(SERVER_ADDRESS);
	sa.sin_port = ntohs(port);

	if (bind(fd, (struct sockaddr*) &sa, sizeof(sa))) {
		goto err;
	}

	return fd;

err:
	closesocket(fd);
	return -1;
}

struct bootp_msg {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint8_t xid[4];
	uint8_t secs[2];
	uint8_t flags[2];
	uint8_t ciaddr[4];
	uint8_t yiaddr[4];
	uint8_t siaddr[4];
	uint8_t giaddr[4];
	uint8_t chaddr[16];
	char sname[64];
	char file[128];
	uint8_t options[64];
};

#define BOOTP_REQUEST 1
#define BOOTP_REPLY 2
#define BOOTP_ETHERNET 1
#define MAC_LEN 6

#define OPT_VENDOR_CLASS 60
#define OPT_ROUTER 3
#define OPT_NETMASK 1
#define OPT_END 255
#define OPT_PAD 0
#define DHCP_MAGIC "\x63\x82\x53\x63"
#define MAGIC_LEN 4

#define TFTP_RRQ 1
#define TFTP_WRQ 2
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ERROR 5

#include <intrin.h>
#pragma intrinsic(_BitScanForward)
static int get_first_set(uint32_t num) {
	unsigned long idx;
	if (!_BitScanForward(&idx, num)) {
		return -1;
	}
	return idx;
}

static void process_bootp(struct bootp *b, char *buf, int sz) {
	if (sz < sizeof(struct bootp_msg)) {
		return;
	}

	int idx = get_first_set(b->clients_free);
	if (idx < 0) {
		return;
	}

	struct bootp_msg *m = (struct bootp_msg*) buf;

	if (m->op != BOOTP_REQUEST
		|| m->htype != BOOTP_ETHERNET
		|| m->hlen != MAC_LEN
		|| m->hops) {
		return;
	}

	struct bootp_client *c = &b->clients[idx];
	memcpy(c->mac, m->chaddr, sizeof(c->mac));

	if (!memcmp(m->options, DHCP_MAGIC, MAGIC_LEN)) {
		uint8_t *opts = m->options + MAGIC_LEN;
		uint8_t *end = (uint8_t*) buf + sz;

		while (opts < end && *opts != OPT_END) {
			int opt = *(opts++);
			if (opt == OPT_PAD || opts == end) {
				continue;
			}
			int olen = *(opts++);
			if (opts + olen > end) {
				return;
			}

			switch (opt) {
			case OPT_VENDOR_CLASS:
				if (olen < sizeof(c->vendor_class) - 1) {
					memcpy(c->vendor_class, opts, olen);
					c->vendor_class[olen] = '\0';
				}
				break;
			}

			opts += olen;
		}
	}

	c->data = NULL;
	c->size = 0;

	b->cb(b->udata, c);
	if (!c->data) {
		return;
	}

	struct bootp_msg r = {0};
	r.op = BOOTP_REPLY;
	r.htype = BOOTP_ETHERNET;
	r.hlen = MAC_LEN;
	memcpy(r.xid, m->xid, sizeof(r.xid));
	memcpy(r.secs, m->secs, sizeof(r.secs));
	memcpy(r.flags, m->flags, sizeof(r.flags));
	put_big_32(r.yiaddr, FIRST_CLIENT + idx);
	put_big_32(r.siaddr, SERVER_ADDRESS);
	put_big_32(r.giaddr, SERVER_ADDRESS);
	memcpy(r.chaddr, m->chaddr, MAC_LEN);
	strcpy(r.file, "firmware");
	strcpy((char*)r.options, DHCP_MAGIC);

	static const uint8_t ropts[] = {
		OPT_NETMASK, 4, 255, 255, 255, 0,
		OPT_ROUTER, 4, SERVER_ADDRESS_SPLIT,
		255,
	};
	memcpy(r.options + MAGIC_LEN, ropts, sizeof(ropts));

	MIB_IPNETROW arp = {0};
	arp.dwIndex = b->interface_id;
	arp.dwPhysAddrLen = MAC_LEN;
	memcpy(arp.bPhysAddr, m->chaddr, MAC_LEN);
	arp.dwAddr = ntohl(FIRST_CLIENT + idx);
	arp.dwType = MIB_IPNET_TYPE_STATIC;
	int ret = CreateIpNetEntry(&arp);
	if (ret == ERROR_OBJECT_ALREADY_EXISTS) {
		ret = SetIpNetEntry(&arp);
	}
	if (ret != NO_ERROR) {
		return;
	}

	struct sockaddr_in sa = {0};
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ntohl(FIRST_CLIENT + idx);
	sa.sin_port = ntohs(BOOTP_CLIENT_PORT);
	if (sendto(b->bootp, (char*) &r, sizeof(r), 0, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
		return;
	}

	b->clients_free &= ~(1 << idx);
}

struct tftp_data {
	uint8_t op[2];
	uint8_t block[2];
	char data[512];
};

static void send_next_block(struct bootp *b, struct bootp_client *c, const struct sockaddr *sa, int sasz) {
	struct tftp_data d;
	put_big_16(d.op, TFTP_DATA);
	put_big_16(d.block, (uint16_t) (c->sent / 512) + 1);
	if (c->sent + 512 <= c->size) {
		memcpy(d.data, c->data + c->sent, sizeof(d.data));
		sendto(b->tftp, (char*) &d, sizeof(d), 0, sa, sasz);
	} else {
		int tosend = (int) (c->size - c->sent);
		memcpy(d.data, c->data + c->sent, tosend);
		sendto(b->tftp, (char*) &d, sizeof(d) - sizeof(d.data) + tosend, 0, sa, sasz);
	}
}

static void process_tftp(struct bootp *b, struct bootp_client *c, uint8_t *u, int sz, const struct sockaddr *sa, int sasz) {
	if (sz < 2) {
		return;
	}
	uint8_t *e = u + sz;
	uint16_t op = big_16(u);

	switch (op) {
	case TFTP_RRQ: {
			uint8_t *fn = u + 2;
			uint8_t *fnend = memchr(fn, 0, e-fn);
			if (!fnend || fnend+1 >= e) {
				return;
			}

			uint8_t *mode = fnend+1;
			uint8_t *mend = memchr(mode, 0, e-mode);
			if (!mend) {
				return;
			}

			if (strcmp((char*)fn, "firmware") || strcmp((char*)mode, "octet")) {
				return;
			}

			send_next_block(b, c, sa, sasz);
		}
		break;

	case TFTP_ACK:
		c->sent += 512;
		if (c->sent <= c->size) {
			send_next_block(b, c, sa, sasz);
		} else {
			c->sent = c->size;
		}
		break;

	default:
		return;
	}
}

static DWORD WINAPI bootp_thread(void *udata) {
	struct bootp *b = (struct bootp*) udata;

	WSAEVENT bootp_recvev = WSACreateEvent();
	WSAEVENT tftp_ev = WSACreateEvent();

	WSAEventSelect(b->bootp, bootp_recvev, FD_READ);
	WSAEventSelect(b->tftp, tftp_ev, FD_READ);

	for (;;) {
		char buf[1500];
		WSANETWORKEVENTS events;
		HANDLE h[3] = {b->stop, bootp_recvev, tftp_ev};

		switch (WaitForMultipleObjects(3, h, FALSE, 5000)) {
		case WAIT_TIMEOUT:
			break;
		case WAIT_OBJECT_0+1:
			if (!WSAEnumNetworkEvents(b->bootp, bootp_recvev, &events) && (events.lNetworkEvents & FD_READ)) {
				for (;;) {
					int w = recv(b->bootp, buf, sizeof(buf), 0);
					if (w < 0) {
						break;
					}

					process_bootp(b, buf, w);
				}
			}
			break;
		case WAIT_OBJECT_0+2:
			if (!WSAEnumNetworkEvents(b->tftp, tftp_ev, &events) && (events.lNetworkEvents & FD_READ)) {
				for (;;) {
					struct sockaddr_in sa;
					int sasz = sizeof(sa);
					int w = recvfrom(b->tftp, buf, sizeof(buf), 0, (struct sockaddr*) &sa, &sasz);
					if (w < 0) {
						break;
					}
					if (sa.sin_family == AF_INET && (ntohl(sa.sin_addr.s_addr) & NETMASK) == SUBNET) {
						int idx = ntohl(sa.sin_addr.s_addr) - FIRST_CLIENT;
						if (0 <= idx && idx < NUM_CLIENTS) {
							struct bootp_client *c = &b->clients[idx];
							process_tftp(b, c, (uint8_t*) buf, w, (struct sockaddr*) &sa, sasz);
						}
					}
				}
			}
			break;

		case WAIT_OBJECT_0:
		default:
			goto end;
		}
	}

end:
	CloseHandle(tftp_ev);
	CloseHandle(bootp_recvev);
	return 0;
}

struct bootp *bootp_start(int interface_id, void *udata, bootp_cb cb) {
	if (setup_server_address(interface_id)) {
		return NULL;
	}

	int bootp = create_socket(67);
	int tftp = create_socket(69);

	if (bootp < 0 || tftp < 0) {
		closesocket(bootp);
		closesocket(tftp);
		return NULL;
	}

	struct bootp *b = (struct bootp*) calloc(1, sizeof(struct bootp));
	b->clients_free = UINT32_MAX;
	b->cb = cb;
	b->udata = udata;
	b->stop = CreateEvent(NULL, FALSE, FALSE, NULL);
	b->bootp = bootp;
	b->tftp = tftp;
	b->interface_id = interface_id;

	DWORD thread_id;
	b->thread = CreateThread(NULL, 0, &bootp_thread, b, 0, &thread_id);

	return b;
}

void bootp_stop(struct bootp *b) {
	SetEvent(b->stop);
	WaitForSingleObject(b->thread, INFINITE);
	CloseHandle(b->thread);
	CloseHandle(b->stop);
	closesocket(b->bootp);
	closesocket(b->tftp);
	free(b);
}


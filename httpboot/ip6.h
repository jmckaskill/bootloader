#pragma once
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#define ETH_IP6 0x86DD

#define IP6_DEFAULT_HEADER 0x60000000
#define IP6_DEFAULT_HOP_LIMIT 255

extern const char hex[];

#define ETH_MCAST      UINT64_C(0x333300000000)
#define ETH_MCAST_MASK UINT64_C(0x0000FFFFFFFF)

#define LINK_LOCAL_SUBNET UINT64_C(0xFE80000000000000)
#define ETH_UL_BIT UINT64_C(0x020000000000)

typedef union {
	uint8_t u[16];
	uint32_t u32[4];
} ip6_addr_t;

typedef union {
	uint8_t u[6];
	uint16_t u16[3];
} mac_addr_t;

extern ip6_addr_t g_my_ip;
extern mac_addr_t g_my_mac;

#pragma pack(push)
#pragma pack(1)
struct ip6_header {
    mac_addr_t eth_dst;
    mac_addr_t eth_src;
    uint8_t eth_type[2];
    uint8_t ip6_header[4];
    uint8_t ip6_length[2];
    uint8_t ip6_next_header;
    uint8_t ip6_hop_limit;
    ip6_addr_t ip6_src;
    ip6_addr_t ip6_dst;
};
#pragma pack(pop)

static_assert(sizeof(struct ip6_header) == 54, "padding");

#define MAX_ETH_SIZE 1514 // doesn't include the trailing crc

#define IP6_UDP 17
struct udp_header {
	uint8_t src_port[2];
	uint8_t dst_port[2];
	uint8_t length[2];
	uint8_t checksum[2];
};

static_assert(sizeof(struct udp_header) == 8, "padding");
#define MAX_UDP_SIZE (MAX_ETH_SIZE - sizeof(struct ip6_header) - sizeof(struct udp_header))

#define PRINT_MAC_LEN (6*3)
#define PRINT_ADDR_LEN (8*5)
char *print_mac(char *buf, const mac_addr_t *v);
char *print_addr(char *buf, const ip6_addr_t *v);
uint16_t ones_checksum(struct ip6_header *ip, int payloadsz);

struct ip6_header *new_ip6_frame(const mac_addr_t *dst_mac, const ip6_addr_t *dst_ip, uint8_t type);
void send_ip6_frame(struct ip6_header *ip, int sz);

static inline void copy_mac(mac_addr_t *dst, const mac_addr_t *src) {
	// we can rely on 16 bit alignment but no more
	uint16_t *to = (uint16_t*) dst->u;
	uint16_t *from = (uint16_t*) src->u;
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
}

static inline void copy_ip6(ip6_addr_t *dst, const ip6_addr_t *src) {
	// ip6 addresses should be 32 bit aligned
	uint32_t *to = (uint32_t*) dst->u;
	uint32_t *from = (uint32_t*) src->u;
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
	to[3] = from[3];
}

static inline int mac_equals(const mac_addr_t *a, const mac_addr_t *b) {
	// we can rely on 16 bit alignment but no more
	uint16_t *to = (uint16_t*) a->u;
	uint16_t *from = (uint16_t*) b->u;
	return to[0] == from[0] && to[1] == from[1] && to[2] == from[2];
}

static inline int ip6_equals(const ip6_addr_t *a, const ip6_addr_t *b) {
	// ip6 addresses should be 32 bit aligned
	uint32_t *to = (uint32_t*) a->u;
	uint32_t *from = (uint32_t*) b->u;
	return to[0] == from[0] && to[1] == from[1] && to[2] == from[2] && to[3] == from[3];
}




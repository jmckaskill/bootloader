#include "tcp.h"

int should_accept_connection(struct ip6_header *ip, uint16_t dst_port) {
    return dst_port == 80;
}

int process_tcp_data(struct tcp_connection *c, const void *msg, int sz) {

}

void test_tcp() {


}


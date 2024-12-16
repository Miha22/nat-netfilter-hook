#include <types.h>

struct nat_entry {
    uint32_t src_ip;
    uint16_t src_port;
    uint32_t public_ip;
    uint16_t public_port;
    uint32_t dest_ip;
    uint16_t dest_port;
    struct hlist_node hnode;
};

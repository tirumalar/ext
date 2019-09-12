#ifndef UDP_PAYLOAD_INSERTER_H
#define UDP_PAYLOAD_INSERTER_H

#include "alt_types.h"

// the entries below marked with "*" are required inputs for start inserter routine
// all entries are returned from the inserter stat routine
typedef struct {
    alt_u32 csr_state;      // out only - csr value
    alt_u32 mac_dst_hi;     // * hi 32 bits of the 48 bit mac destination address
    alt_u16 mac_dst_lo;     // * lo 16 bits of the 48 bit mac destination address
    alt_u32 mac_src_hi;     // * hi 32 bits of the 48 bit mac source address
    alt_u16 mac_src_lo;     // * lo 16 bits of the 48 bit mac source address
    alt_u32 ip_src;         // * IP address of source
    alt_u32 ip_dst;         // * IP address of destination
    alt_u16 udp_src;        // * UDP source port
    alt_u16 udp_dst;        // * UDP destination port
    alt_u32 packet_count;   // packet counter value
} UDP_INS_STATS;

int start_udp_payload_inserter(void *base, UDP_INS_STATS *stats);
int stop_udp_payload_inserter(void *base);
int is_udp_payload_inserter_running(void *base);
int wait_until_udp_payload_inserter_stops_running(void *base);
int check_udp_payload_inserter_error(void *base);
int get_udp_payload_inserter_stats(void *base, UDP_INS_STATS *stats);

#endif /*UDP_PAYLOAD_INSERTER_H*/

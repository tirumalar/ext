#include "udp_payload_inserter_regs.h"
#include "udp_payload_inserter.h"

//
// udp payload inserter utility routines
//

int start_udp_payload_inserter(void *base, UDP_INS_STATS *stats) {
    
    alt_u32 current_csr;

    // is the packet generator already running?
    current_csr = UDP_PAYLOAD_INSERTER_RD_CSR(base);
    if(current_csr & UDP_PAYLOAD_INSERTER_CSR_GO_BIT_MASK) {
        return 1;
    }
    if(current_csr & UDP_PAYLOAD_INSERTER_CSR_RUNNING_BIT_MASK) {
        return 2;
    }
    
    // clear the counter    
    UDP_PAYLOAD_INSERTER_CLEAR_PACKET_COUNTER(base);
    
    // write the parameter registers
    UDP_PAYLOAD_INSERTER_WR_MAC_DST_HI  (base, stats->mac_dst_hi);
    UDP_PAYLOAD_INSERTER_WR_MAC_DST_LO  (base, stats->mac_dst_lo);
    UDP_PAYLOAD_INSERTER_WR_MAC_SRC_HI  (base, stats->mac_src_hi);
    UDP_PAYLOAD_INSERTER_WR_MAC_SRC_LO  (base, stats->mac_src_lo);
    UDP_PAYLOAD_INSERTER_WR_IP_SRC      (base, stats->ip_src);
    UDP_PAYLOAD_INSERTER_WR_IP_DST      (base, stats->ip_dst);
    UDP_PAYLOAD_INSERTER_WR_UDP_PORTS   (base, (alt_u32)(stats->udp_src << 16) | (alt_u32)(stats->udp_dst));

    // and set the go bit
    UDP_PAYLOAD_INSERTER_WR_CSR(base, UDP_PAYLOAD_INSERTER_CSR_GO_BIT_MASK);
    
    return 0;
    
}

int stop_udp_payload_inserter(void *base) {
    
    // is the peripheral already stopped?
    if(!(UDP_PAYLOAD_INSERTER_RD_CSR(base) & UDP_PAYLOAD_INSERTER_CSR_GO_BIT_MASK)) {
        return 1;
    }

    // clear the go bit
    UDP_PAYLOAD_INSERTER_WR_CSR(base, 0);
    
    return 0;
}

int is_udp_payload_inserter_running(void *base) {
    
    // is the peripheral running?
    if((UDP_PAYLOAD_INSERTER_RD_CSR(base) & UDP_PAYLOAD_INSERTER_CSR_RUNNING_BIT_MASK)) {
        return 1;
    }

    return 0;
}

int wait_until_udp_payload_inserter_stops_running(void *base) {
    
    // wait until peripheral stops running?
    while(is_udp_payload_inserter_running(base));

    return 0;
}

int check_udp_payload_inserter_error(void *base) {
    
    // is the peripheral in error state?
    if((UDP_PAYLOAD_INSERTER_RD_CSR(base) & UDP_PAYLOAD_INSERTER_CSR_ERROR_BIT_MASK)) {
        return 1;
    }

    return 0;
}

int get_udp_payload_inserter_stats(void *base, UDP_INS_STATS *stats) {
    
    stats->csr_state    = UDP_PAYLOAD_INSERTER_RD_CSR(base);
    stats->mac_dst_hi   = UDP_PAYLOAD_INSERTER_RD_MAC_DST_HI(base);
    stats->mac_dst_lo   = UDP_PAYLOAD_INSERTER_RD_MAC_DST_LO(base);
    stats->mac_src_hi   = UDP_PAYLOAD_INSERTER_RD_MAC_SRC_HI(base);
    stats->mac_src_lo   = UDP_PAYLOAD_INSERTER_RD_MAC_SRC_LO(base);
    stats->ip_src       = UDP_PAYLOAD_INSERTER_RD_IP_SRC(base);
    stats->ip_dst       = UDP_PAYLOAD_INSERTER_RD_IP_DST(base);
    stats->udp_src      = (UDP_PAYLOAD_INSERTER_RD_UDP_PORTS(base) & UDP_PAYLOAD_INSERTER_UDP_SRC_MASK) >> UDP_PAYLOAD_INSERTER_UDP_SRC_OFST;
    stats->udp_dst      = (UDP_PAYLOAD_INSERTER_RD_UDP_PORTS(base) & UDP_PAYLOAD_INSERTER_UDP_DST_MASK) >> UDP_PAYLOAD_INSERTER_UDP_DST_OFST;
    stats->packet_count = UDP_PAYLOAD_INSERTER_RD_PACKET_COUNTER(base);
    
    return 0;
}

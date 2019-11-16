#ifndef UDP_PAYLOAD_INSERTER_REGS_H
#define UDP_PAYLOAD_INSERTER_REGS_H

#include "io.h"

// UDP PAYLOAD INSERTER ACCESS MACROS

#define UDP_PAYLOAD_INSERTER_RD_CSR(base)                   IORD(base, 0)
#define UDP_PAYLOAD_INSERTER_WR_CSR(base, data)             IOWR(base, 0, data)

#define UDP_PAYLOAD_INSERTER_CSR_GO_BIT_MASK                (0x01)
#define UDP_PAYLOAD_INSERTER_CSR_GO_BIT_OFST                (0)
#define UDP_PAYLOAD_INSERTER_CSR_RUNNING_BIT_MASK           (0x02)
#define UDP_PAYLOAD_INSERTER_CSR_RUNNING_BIT_OFST           (1)
#define UDP_PAYLOAD_INSERTER_CSR_ERROR_BIT_MASK             (0x04)
#define UDP_PAYLOAD_INSERTER_CSR_ERROR_BIT_OFST             (2)

#define UDP_PAYLOAD_INSERTER_RD_MAC_DST_HI(base)            IORD(base, 1)
#define UDP_PAYLOAD_INSERTER_WR_MAC_DST_HI(base, data)      IOWR(base, 1, data)

#define UDP_PAYLOAD_INSERTER_MAC_DST_HI_MASK                (0xFFFFFFFF)
#define UDP_PAYLOAD_INSERTER_MAC_DST_HI_OFST                (0)

#define UDP_PAYLOAD_INSERTER_RD_MAC_DST_LO(base)            IORD(base, 2)
#define UDP_PAYLOAD_INSERTER_WR_MAC_DST_LO(base, data)      IOWR(base, 2, data)

#define UDP_PAYLOAD_INSERTER_MAC_DST_LO_MASK                (0xFFFF)
#define UDP_PAYLOAD_INSERTER_MAC_DST_LO_OFST                (0)

#define UDP_PAYLOAD_INSERTER_RD_MAC_SRC_HI(base)            IORD(base, 3)
#define UDP_PAYLOAD_INSERTER_WR_MAC_SRC_HI(base, data)      IOWR(base, 3, data)

#define UDP_PAYLOAD_INSERTER_MAC_SRC_HI_MASK                (0xFFFFFFFF)
#define UDP_PAYLOAD_INSERTER_MAC_SRC_HI_OFST                (0)

#define UDP_PAYLOAD_INSERTER_RD_MAC_SRC_LO(base)            IORD(base, 4)
#define UDP_PAYLOAD_INSERTER_WR_MAC_SRC_LO(base, data)      IOWR(base, 4, data)

#define UDP_PAYLOAD_INSERTER_MAC_SRC_LO_MASK                (0xFFFF)
#define UDP_PAYLOAD_INSERTER_MAC_SRC_LO_OFST                (0)

#define UDP_PAYLOAD_INSERTER_RD_IP_SRC(base)                IORD(base, 5)
#define UDP_PAYLOAD_INSERTER_WR_IP_SRC(base, data)          IOWR(base, 5, data)

#define UDP_PAYLOAD_INSERTER_IP_SRC_MASK                    (0xFFFFFFFF)
#define UDP_PAYLOAD_INSERTER_IP_SRC_OFST                    (0)

#define UDP_PAYLOAD_INSERTER_RD_IP_DST(base)                IORD(base, 6)
#define UDP_PAYLOAD_INSERTER_WR_IP_DST(base, data)          IOWR(base, 6, data)

#define UDP_PAYLOAD_INSERTER_IP_DST_MASK                    (0xFFFFFFFF)
#define UDP_PAYLOAD_INSERTER_IP_DST_OFST                    (0)

#define UDP_PAYLOAD_INSERTER_RD_UDP_PORTS(base)             IORD(base, 7)
#define UDP_PAYLOAD_INSERTER_WR_UDP_PORTS(base, data)       IOWR(base, 7, data)

#define UDP_PAYLOAD_INSERTER_UDP_DST_MASK                   (0x0000FFFF)
#define UDP_PAYLOAD_INSERTER_UDP_DST_OFST                   (0)
#define UDP_PAYLOAD_INSERTER_UDP_SRC_MASK                   (0xFFFF0000)
#define UDP_PAYLOAD_INSERTER_UDP_SRC_OFST                   (16)

#define UDP_PAYLOAD_INSERTER_RD_PACKET_COUNTER(base)        IORD(base, 8)
#define UDP_PAYLOAD_INSERTER_CLEAR_PACKET_COUNTER(base)     IOWR(base, 8, 0)

#define UDP_PAYLOAD_INSERTER_PACKET_COUNTER_MASK            (0xFFFFFFFF)
#define UDP_PAYLOAD_INSERTER_PACKET_COUNTER_OFST            (0)

#endif /*UDP_PAYLOAD_INSERTER_REGS_H*/

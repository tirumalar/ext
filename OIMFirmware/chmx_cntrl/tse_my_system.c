#ifdef ALT_INICHE
    #include "ipport.h"
#endif

#include "system.h"
#include "altera_avalon_tse.h"
#include "altera_avalon_tse_system_info.h"

alt_tse_system_info tse_mac_device[MAXNETS] = {
		TSE_SYSTEM_EXT_MEM_NO_SHARED_FIFO(ETH_TSE_0, 0, SGDMA_TX, SGDMA_RX, TSE_PHY_AUTO_ADDRESS, 0, ONCHIP_MEMORY2_1)
		
};

//result = getPHYSpeed(tse[iface].mi.base);

/** Function pointer to read the link status from the PHY specific status register
  * Use this function pointer if the PHY is using different format to store link information in PHY specific status register
  * The above _location variable will not be used if this function pointer is not NULL
  * Table below show the format of the return value required by TSE driver PHY detection
  * ----------------------------------------------------------------------------------
  * |  BIT  | Value: Description                                                     |
  * ----------------------------------------------------------------------------------
  * | 31-17 | Reserved                                                               |
  * |   16  | 1: Error:Invalid speed read from PHY                                   |
  * | 15- 4 | Reserved                                                               |
  * |    3  | 1: 10 Mbps link                                                        |
  * |    2  | 1: 100 Mbps link                                                       |
  * |    1  | 1: 1000 Mbps link                                                      |
  * |    0  | 1: Full Duplex                    0: Half Duplex                       |
  * ----------------------------------------------------------------------------------
  */

alt_u32 Microchip_link_status_read(np_tse_mac *pmac)
    {

    int stat = IORD(&pmac->mdio0.STATUS, 0);
	printf("mdio %x\n", IORD(&pmac->mdio0.STATUS, 0));
  if (stat&0x4)
	return 3;
  else
      return 0;
}

alt_32 Microchip_phy_cfg(np_tse_mac *pmac) {

	alt_u16 dat;

    /* If there is no link yet, we enable auto crossover and reset the PHY */

        printf("resetting phy chip\n");
        dat = IORD(&pmac->mdio1.CONTROL, 0);
        IOWR(&pmac->mdio1.CONTROL, 0, dat | PCS_CTL_sw_reset);
/*
        IOWR(&pmac->mdio1.regd, 0, 2); // device 2
        IOWR(&pmac->mdio1.rege, 0, 8); // device reg 8
        IOWR(&pmac->mdio1.regd, 0,0x4002); // set for read
        dat = IORD(&pmac->mdio1.rege, 0);
        printf("Clock scew %x\n",dat);
        IOWR(&pmac->mdio1.rege, 0, (dat&0xffe0)|0x1f);
        dat = IORD(&pmac->mdio1.rege, 0);
        printf("Clock scew %x\n",dat);

*/
    return 0;
}

alt_tse_phy_profile microchip = {"mICROCHIP",  /* National DP83848C                                          */
                       0x885,                   /* OUI                                                        */
                       0X22,                 /* Vender Model Number                                        */
                       0x2,                   /* Model Revision Number                                      */
                       0,                              /* Location of Status Register (ignored)                      */
                       0,                              /* Location of Speed Status    (ignored)                      */
                       0,                              /* Location of Duplex Status   (ignored)                      */
                       0,                              /* Location of Link Status     (ignored)                      */
                       Microchip_phy_cfg,                              /* No function pointer configure National DP83848C            */
					   &Microchip_link_status_read      /* Function pointer to read from PHY specific status register */
                      };

/* add supported PHY to profile */
void do_mic_init(void)
{
alt_tse_phy_add_profile(&microchip);
}


#if 0
alt_32 alt_tse_phy_add_profile_default() {

    /* supported PHY definition */

    /* ------------------------------ */
    /* Marvell PHY on PHYWORKX board  */
    /* ------------------------------ */

    alt_tse_phy_profile MV88E1111 = {"Marvell 88E1111",      /* Marvell 88E1111                                  */
                            MV88E1111_OUI,          /* OUI                                                           */
                            MV88E1111_MODEL,        /* Vender Model Number                                           */
                            MV88E1111_REV,          /* Model Revision Number                                         */
                            0x11,                   /* Location of Status Register                                   */
                            14,                     /* Location of Speed Status                                      */
                            13,                     /* Location of Duplex Status                                     */
                            10,                     /* Location of Link Status                                       */
                            &marvell_phy_cfg        /* Function pointer to configure Marvell PHY                     */
                           };


    /* ---------------------------------- */
    /* Marvell Quad PHY on PHYWORKX board */
    /* ---------------------------------- */

    alt_tse_phy_profile MV88E1145 = {"Marvell Quad PHY 88E1145",      /* Marvell 88E1145                                  */
                            MV88E1145_OUI,                   /* OUI                                                           */
                            MV88E1145_MODEL,                 /* Vender Model Number                                           */
                            MV88E1145_REV,                   /* Model Revision Number                                         */
                            0x11,                            /* Location of Status Register                                   */
                            14,                              /* Location of Speed Status                                      */
                            13,                              /* Location of Duplex Status                                     */
                            10,                              /* Location of Link Status                                       */
                            &marvell_phy_cfg                 /* Function pointer to configure Marvell PHY                     */
                           };

    /* ------------------------------ */
    /* National PHY on PHYWORKX board */
    /* ------------------------------ */

    alt_tse_phy_profile DP83865 = {"National DP83865",     /* National DP83865                                 */
                          DP83865_OUI,            /* OUI                                              */
                          DP83865_MODEL,          /* Vender Model Number                              */
                          DP83865_REV,            /* Model Revision Number                            */
                          0x11,                   /* Location of Status Register                      */
                          3,                      /* Location of Speed Status                         */
                          1,                      /* Location of Duplex Status                        */
                          2                       /* Location of Link Status                          */
                         };

    /* -------------------------------------- */
    /* National 10/100 PHY on PHYWORKX board  */
    /* -------------------------------------- */

    alt_tse_phy_profile DP83848C = {"National DP83848C",  /* National DP83848C                                          */
                           DP83848C_OUI,                   /* OUI                                                        */
                           DP83848C_MODEL,                 /* Vender Model Number                                        */
                           DP83848C_REV,                   /* Model Revision Number                                      */
                           0,                              /* Location of Status Register (ignored)                      */
                           0,                              /* Location of Speed Status    (ignored)                      */
                           0,                              /* Location of Duplex Status   (ignored)                      */
                           0,                              /* Location of Link Status     (ignored)                      */
						   0,                              /* No function pointer configure National DP83848C            */
						   &DP83848C_link_status_read      /* Function pointer to read from PHY specific status register */
                          };

    /* add supported PHY to profile */
    alt_tse_phy_add_profile(&MV88E1111);
    alt_tse_phy_add_profile(&MV88E1145);
    alt_tse_phy_add_profile(&DP83865);
    alt_tse_phy_add_profile(&DP83848C);

    {
                                	extern alt_tse_phy_profile microchip;
                                	alt_tse_phy_add_profile(&microchip);
                                }

    return 5;
}
#endif

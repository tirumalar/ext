/*
 * Registers for the ADP8860 LED charge pump
 */

#ifndef ADP8860_REGS_H
#define ADP8860_REGS_H

/* Manufacturer and Device ID - MFDVID */
#define ADP_MFDVID_REG	0x00

#define MFDVID_MFG_ID(x)	(((x) >> 4) & 0x0f)
#define MFDVID_DEV_ID(x)	((x) & 0x0f)

/* Mode Control Register - MDCR */
#define ADP_MDCR_REG	0x01

#define MDCR_nSTBY	(1 << 5)
#define MDCR_SIS_EN	(1 << 2) 
#define MDCR_BLEN	(1 << 0)


/* Backlight Sink Enable - BLSEN */
#define ADP_BLSEN_REG	0x05

#define BLSEN_D1EN	(1 << 0)
#define BLSEN_D2EN	(1 << 1)
#define BLSEN_D3EN	(1 << 2)
#define BLSEN_D4EN	(1 << 3)
#define BLSEN_D5EN	(1 << 4)
#define BLSEN_D6EN	(1 << 5)
#define BLSEN_D7EN	(1 << 6)


/* Independent Sink Current Control - ISCC */
#define ADP_ISCC_REG	0x10

#define ISCC_SC1_EN	(1 << 0)
#define ISCC_SC2_EN	(1 << 1)
#define ISCC_SC3_EN	(1 << 2)
#define ISCC_SC4_EN	(1 << 3)
#define ISCC_SC5_EN	(1 << 4)
#define ISCC_SC6_EN	(1 << 5)
#define ISCC_SC7_EN	(1 << 6)


/* Sink Current Registers - ISCx */
#define ADP_ISC3_REG	0x18
#define ADP_ISC2_REG	0x19
#define ADP_ISC1_REG	0x1A

#define ISC_VAL(x)	((x) & 0x7f)


#endif



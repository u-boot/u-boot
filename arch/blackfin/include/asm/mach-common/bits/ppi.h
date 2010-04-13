/*
 * PPI Masks
 */

#ifndef __BFIN_PERIPHERAL_PPI__
#define __BFIN_PERIPHERAL_PPI__

/* PPI_CONTROL Masks */
#define PORT_EN			0x0001	/* PPI Port Enable */
#define PORT_DIR		0x0002	/* PPI Port Direction */
#define XFR_TYPE		0x000C	/* PPI Transfer Type */
#define PORT_CFG		0x0030	/* PPI Port Configuration */
#define FLD_SEL			0x0040	/* PPI Active Field Select */
#define PACK_EN			0x0080	/* PPI Packing Mode */
#define DMA32			0x0100	/* PPI 32-bit DMA Enable */
#define SKIP_EN			0x0200	/* PPI Skip Element Enable */
#define SKIP_EO			0x0400	/* PPI Skip Even/Odd Elements */
#define DLENGTH			0x3800	/* PPI Data Length */
#define DLEN_8			0x0000	/* Data Length = 8 Bits */
#define DLEN_10			0x0800	/* Data Length = 10 Bits */
#define DLEN_11			0x1000	/* Data Length = 11 Bits */
#define DLEN_12			0x1800	/* Data Length = 12 Bits */
#define DLEN_13			0x2000	/* Data Length = 13 Bits */
#define DLEN_14			0x2800	/* Data Length = 14 Bits */
#define DLEN_15			0x3000	/* Data Length = 15 Bits */
#define DLEN_16			0x3800	/* Data Length = 16 Bits */
#define POLC			0x4000	/* PPI Clock Polarity */
#define POLS			0x8000	/* PPI Frame Sync Polarity */

/* PPI_STATUS Masks */
#define FLD			0x0400	/* Field Indicator */
#define FT_ERR			0x0800	/* Frame Track Error */
#define OVR			0x1000	/* FIFO Overflow Error */
#define UNDR			0x2000	/* FIFO Underrun Error */
#define ERR_DET			0x4000	/* Error Detected Indicator */
#define ERR_NCOR		0x8000	/* Error Not Corrected Indicator */

#endif

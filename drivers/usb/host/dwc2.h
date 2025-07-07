/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef __DWC2_H__
#define __DWC2_H__

/* Host controller specific */
#define DWC2_HC_PID_DATA0		0
#define DWC2_HC_PID_DATA2		1
#define DWC2_HC_PID_DATA1		2
#define DWC2_HC_PID_MDATA		3
#define DWC2_HC_PID_SETUP		3

/* roothub.a masks */
#define RH_A_NDP	GENMASK(7, 0)	/* number of downstream ports */
#define RH_A_PSM	BIT(8)		/* power switching mode */
#define RH_A_NPS	BIT(9)		/* no power switching */
#define RH_A_DT		BIT(10)		/* device type (mbz) */
#define RH_A_OCPM	BIT(11)		/* over current protection mode */
#define RH_A_NOCP	BIT(12)		/* no over current protection */
#define RH_A_POTPGT	GENMASK(31, 24)	/* power on to power good time */

/* roothub.b masks */
#define RH_B_DR		0x0000ffff	/* device removable flags */
#define RH_B_PPCM	0xffff0000	/* port power control mask */

/* Default driver configuration */
#define DWC2_DMA_ENABLE
#define DWC2_DMA_BURST_SIZE		32	/* DMA burst len */
#undef DWC2_DFLT_SPEED_FULL		/* Do not force DWC2 to FS */
#define DWC2_ENABLE_DYNAMIC_FIFO		/* Runtime FIFO size detect */
#define DWC2_MAX_CHANNELS		16	/* Max # of EPs */
#define DWC2_HOST_RX_FIFO_SIZE		(516 + DWC2_MAX_CHANNELS)
#define DWC2_HOST_NPERIO_TX_FIFO_SIZE	0x100	/* nPeriodic TX FIFO */
#define DWC2_HOST_PERIO_TX_FIFO_SIZE	0x200	/* Periodic TX FIFO */
#define DWC2_MAX_TRANSFER_SIZE		65535
#define DWC2_MAX_PACKET_COUNT		511

#define DWC2_PHY_TYPE_FS		0
#define DWC2_PHY_TYPE_UTMI		1
#define DWC2_PHY_TYPE_ULPI		2
#define DWC2_PHY_TYPE		DWC2_PHY_TYPE_UTMI	/* PHY type */
#ifndef DWC2_UTMI_WIDTH
#define DWC2_UTMI_WIDTH		8	/* UTMI bus width (8/16) */
#endif

#undef DWC2_PHY_ULPI_DDR			/* ULPI PHY uses DDR mode */
#define DWC2_PHY_ULPI_EXT_VBUS		/* ULPI PHY controls VBUS */
#undef DWC2_I2C_ENABLE			/* Enable I2C */
#undef DWC2_ULPI_FS_LS			/* ULPI is FS/LS */
#undef DWC2_TS_DLINE			/* External DLine pulsing */
#undef DWC2_THR_CTL			/* Threshold control */
#define DWC2_TX_THR_LENGTH		64
#undef DWC2_IC_USB_CAP			/* IC Cap */

#endif	/* __DWC2_H__ */

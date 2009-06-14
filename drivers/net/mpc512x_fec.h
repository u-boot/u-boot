/*
 * (C) Copyright 2003 - 2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Derived from the MPC8xx driver's header file.
 */

#ifndef __MPC512X_FEC_H
#define __MPC512X_FEC_H

#include <common.h>

/* Receive & Transmit Buffer Descriptor definitions */
typedef struct BufferDescriptor {
	u16 status;
	u16 dataLength;
	u32 dataPointer;
} FEC_RBD;

typedef struct {
	u16 status;
	u16 dataLength;
	u32 dataPointer;
} FEC_TBD;

/* private structure */
typedef enum {
	SEVENWIRE,			/* 7-wire       */
	MII10,				/* MII 10Mbps   */
	MII100				/* MII 100Mbps  */
} xceiver_type;

/* BD Numer definitions */
#define FEC_TBD_NUM		48	/* The user can adjust this value */
#define FEC_RBD_NUM		32	/* The user can adjust this value */

/* packet size limit */
#define FEC_MAX_FRAME_LEN	1522	/* recommended default value */

/* Buffer size must be evenly divisible by 16 */
#define FEC_BUFFER_SIZE		((FEC_MAX_FRAME_LEN + 0x10) & (~0xf))

typedef struct {
	u8 frame[FEC_BUFFER_SIZE];
} mpc512x_frame;

typedef struct {
	FEC_RBD rbd[FEC_RBD_NUM];			/* RBD ring */
	FEC_TBD tbd[FEC_TBD_NUM];			/* TBD ring */
	mpc512x_frame recv_frames[FEC_RBD_NUM];		/* receive buff */
} mpc512x_buff_descs;

typedef struct {
	volatile fec512x_t *eth;
	xceiver_type xcv_type;		/* transceiver type */
	mpc512x_buff_descs *bdBase;	/* BD rings and recv buffer */
	u16 rbdIndex;			/* next receive BD to read */
	u16 tbdIndex;			/* next transmit BD to send */
	u16 usedTbdIndex;		/* next transmit BD to clean */
	u16 cleanTbdNum;		/* the number of available transmit BDs */
} mpc512x_fec_priv;

/* RBD bits definitions */
#define FEC_RBD_EMPTY		0x8000	/* Buffer is empty */
#define FEC_RBD_WRAP		0x2000	/* Last BD in ring */
#define FEC_RBD_LAST		0x0800	/* Buffer is last in frame(useless) */
#define FEC_RBD_MISS		0x0100	/* Miss bit for prom mode */
#define FEC_RBD_BC		0x0080	/* The received frame is broadcast frame */
#define FEC_RBD_MC		0x0040	/* The received frame is multicast frame */
#define FEC_RBD_LG		0x0020	/* Frame length violation */
#define FEC_RBD_NO		0x0010	/* Nonoctet align frame */
#define FEC_RBD_SH		0x0008	/* Short frame */
#define FEC_RBD_CR		0x0004	/* CRC error */
#define FEC_RBD_OV		0x0002	/* Receive FIFO overrun */
#define FEC_RBD_TR		0x0001	/* Frame is truncated */
#define FEC_RBD_ERR		(FEC_RBD_LG | FEC_RBD_NO | FEC_RBD_CR | \
				FEC_RBD_OV | FEC_RBD_TR)

/* TBD bits definitions */
#define FEC_TBD_READY		0x8000	/* Buffer is ready */
#define FEC_TBD_WRAP		0x2000	/* Last BD in ring */
#define FEC_TBD_LAST		0x0800	/* Buffer is last in frame */
#define FEC_TBD_TC		0x0400	/* Transmit the CRC */
#define FEC_TBD_ABC		0x0200	/* Append bad CRC */

/* MII-related definitios */
#define FEC_MII_DATA_ST		0x40000000	/* Start of frame delimiter */
#define FEC_MII_DATA_OP_RD	0x20000000	/* Perform a read operation */
#define FEC_MII_DATA_OP_WR	0x10000000	/* Perform a write operation */
#define FEC_MII_DATA_PA_MSK	0x0f800000	/* PHY Address field mask */
#define FEC_MII_DATA_RA_MSK	0x007c0000	/* PHY Register field mask */
#define FEC_MII_DATA_TA		0x00020000	/* Turnaround */
#define FEC_MII_DATA_DATAMSK	0x0000ffff	/* PHY data field */

#define FEC_MII_DATA_RA_SHIFT	18	/* MII Register address bits */
#define FEC_MII_DATA_PA_SHIFT	23	/* MII PHY address bits */

#endif	/* __MPC512X_FEC_H */

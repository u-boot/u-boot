/*
 * (C) Copyright 2003 - 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Derived from the MPC8xx driver's header file.
 */

#ifndef __MPC512X_FEC_H
#define __MPC512X_FEC_H

#include <common.h>
#include <mpc512x.h>

typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef struct ethernet_register_set {

/* [10:2]addr = 00 */

/*  Control and status Registers (offset 000-1FF) */

	volatile uint32 fec_id;			/* MBAR_ETH + 0x000 */
	volatile uint32 ievent;			/* MBAR_ETH + 0x004 */
	volatile uint32 imask;			/* MBAR_ETH + 0x008 */

	volatile uint32 RES0[1];		/* MBAR_ETH + 0x00C */
	volatile uint32 r_des_active;		/* MBAR_ETH + 0x010 */
	volatile uint32 x_des_active;		/* MBAR_ETH + 0x014 */
	
	volatile uint32 RES1[3];		/* MBAR_ETH + 0x018-020 */
	volatile uint32 ecntrl;			/* MBAR_ETH + 0x024 */

	volatile uint32 RES2[6];		/* MBAR_ETH + 0x028-03C */
	volatile uint32 mii_data;		/* MBAR_ETH + 0x040 */
	volatile uint32 mii_speed;		/* MBAR_ETH + 0x044 */

	volatile uint32 RES3[7];		/* MBAR_ETH + 0x048-060 */
	volatile uint32 mib_control;		/* MBAR_ETH + 0x064 */

	volatile uint32 RES4[7];		/* MBAR_ETH + 0x068-80 */
	volatile uint32 r_cntrl;		/* MBAR_ETH + 0x084 */
	volatile uint32 r_hash;			/* MBAR_ETH + 0x088 */
	
	volatile uint32 RES5[14];		/* MBAR_ETH + 0x08c-0C0 */
	volatile uint32 x_cntrl;		/* MBAR_ETH + 0x0C4 */
	
	volatile uint32 RES6[7];		/* MBAR_ETH + 0x0C8-0E0 */
	volatile uint32 paddr1;			/* MBAR_ETH + 0x0E4 */
	volatile uint32 paddr2;			/* MBAR_ETH + 0x0E8 */
	volatile uint32 op_pause;		/* MBAR_ETH + 0x0EC */

	volatile uint32 RES7[10];		/* MBAR_ETH + 0x0F0-114 */
	volatile uint32 iaddr1;			/* MBAR_ETH + 0x118 */
	volatile uint32 iaddr2;			/* MBAR_ETH + 0x11C */
	volatile uint32 gaddr1;			/* MBAR_ETH + 0x120 */
	volatile uint32 gaddr2;			/* MBAR_ETH + 0x124 */

	volatile uint32 RES8[6];		/* MBAR_ETH + 0x128-13C */
	volatile uint32 fifo_id;		/* MBAR_ETH + 0x140 */
	volatile uint32 x_wmrk;			/* MBAR_ETH + 0x144 */
	volatile uint32 RES9[1];		/* MBAR_ETH + 0x148 */
	volatile uint32 r_bound;		/* MBAR_ETH + 0x14C */
	volatile uint32 r_fstart;		/* MBAR_ETH + 0x150 */
	
	volatile uint32 RES10[11];		/* MBAR_ETH + 0x154-17C */
	volatile uint32 r_des_start;		/* MBAR_ETH + 0x180 */
	volatile uint32 x_des_start;		/* MBAR_ETH + 0x184 */
	volatile uint32 r_buff_size;		/* MBAR_ETH + 0x188 */
	volatile uint32 RES11[26];		/* MBAR_ETH + 0x18C-1F0 */
	volatile uint32 dma_control;		/* MBAR_ETH + 0x1F4 */
	volatile uint32 RES12[2];		/* MBAR_ETH + 0x1F8-1FC */

/*  MIB COUNTERS (Offset 200-2FF) */

	volatile uint32 rmon_t_drop;		/* MBAR_ETH + 0x200 */
	volatile uint32 rmon_t_packets;		/* MBAR_ETH + 0x204 */
	volatile uint32 rmon_t_bc_pkt;		/* MBAR_ETH + 0x208 */
	volatile uint32 rmon_t_mc_pkt;		/* MBAR_ETH + 0x20C */
	volatile uint32 rmon_t_crc_align;	/* MBAR_ETH + 0x210 */
	volatile uint32 rmon_t_undersize;	/* MBAR_ETH + 0x214 */
	volatile uint32 rmon_t_oversize;	/* MBAR_ETH + 0x218 */
	volatile uint32 rmon_t_frag;		/* MBAR_ETH + 0x21C */
	volatile uint32 rmon_t_jab;		/* MBAR_ETH + 0x220 */
	volatile uint32 rmon_t_col;		/* MBAR_ETH + 0x224 */
	volatile uint32 rmon_t_p64;		/* MBAR_ETH + 0x228 */
	volatile uint32 rmon_t_p65to127;	/* MBAR_ETH + 0x22C */
	volatile uint32 rmon_t_p128to255;	/* MBAR_ETH + 0x230 */
	volatile uint32 rmon_t_p256to511;	/* MBAR_ETH + 0x234 */
	volatile uint32 rmon_t_p512to1023;	/* MBAR_ETH + 0x238 */
	volatile uint32 rmon_t_p1024to2047;	/* MBAR_ETH + 0x23C */
	volatile uint32 rmon_t_p_gte2048;	/* MBAR_ETH + 0x240 */
	volatile uint32 rmon_t_octets;		/* MBAR_ETH + 0x244 */
	volatile uint32 ieee_t_drop;		/* MBAR_ETH + 0x248 */
	volatile uint32 ieee_t_frame_ok;	/* MBAR_ETH + 0x24C */
	volatile uint32 ieee_t_1col;		/* MBAR_ETH + 0x250 */
	volatile uint32 ieee_t_mcol;		/* MBAR_ETH + 0x254 */
	volatile uint32 ieee_t_def;		/* MBAR_ETH + 0x258 */
	volatile uint32 ieee_t_lcol;		/* MBAR_ETH + 0x25C */
	volatile uint32 ieee_t_excol;		/* MBAR_ETH + 0x260 */
	volatile uint32 ieee_t_macerr;		/* MBAR_ETH + 0x264 */
	volatile uint32 ieee_t_cserr;		/* MBAR_ETH + 0x268 */
	volatile uint32 ieee_t_sqe;		/* MBAR_ETH + 0x26C */
	volatile uint32 t_fdxfc;		/* MBAR_ETH + 0x270 */
	volatile uint32 ieee_t_octets_ok;	/* MBAR_ETH + 0x274 */

	volatile uint32 RES13[2];		/* MBAR_ETH + 0x278-27C */
	volatile uint32 rmon_r_drop;		/* MBAR_ETH + 0x280 */
	volatile uint32 rmon_r_packets;		/* MBAR_ETH + 0x284 */
	volatile uint32 rmon_r_bc_pkt;		/* MBAR_ETH + 0x288 */
	volatile uint32 rmon_r_mc_pkt;		/* MBAR_ETH + 0x28C */
	volatile uint32 rmon_r_crc_align;	/* MBAR_ETH + 0x290 */
	volatile uint32 rmon_r_undersize;	/* MBAR_ETH + 0x294 */
	volatile uint32 rmon_r_oversize;	/* MBAR_ETH + 0x298 */
	volatile uint32 rmon_r_frag;		/* MBAR_ETH + 0x29C */
	volatile uint32 rmon_r_jab;		/* MBAR_ETH + 0x2A0 */

	volatile uint32 rmon_r_resvd_0;		/* MBAR_ETH + 0x2A4 */

	volatile uint32 rmon_r_p64;		/* MBAR_ETH + 0x2A8 */
	volatile uint32 rmon_r_p65to127;	/* MBAR_ETH + 0x2AC */
	volatile uint32 rmon_r_p128to255;	/* MBAR_ETH + 0x2B0 */
	volatile uint32 rmon_r_p256to511;	/* MBAR_ETH + 0x2B4 */
	volatile uint32 rmon_r_p512to1023;	/* MBAR_ETH + 0x2B8 */
	volatile uint32 rmon_r_p1024to2047;	/* MBAR_ETH + 0x2BC */
	volatile uint32 rmon_r_p_gte2048;	/* MBAR_ETH + 0x2C0 */
	volatile uint32 rmon_r_octets;		/* MBAR_ETH + 0x2C4 */
	volatile uint32 ieee_r_drop;		/* MBAR_ETH + 0x2C8 */
	volatile uint32 ieee_r_frame_ok;	/* MBAR_ETH + 0x2CC */
	volatile uint32 ieee_r_crc;		/* MBAR_ETH + 0x2D0 */
	volatile uint32 ieee_r_align;		/* MBAR_ETH + 0x2D4 */
	volatile uint32 r_macerr;		/* MBAR_ETH + 0x2D8 */
	volatile uint32 r_fdxfc;		/* MBAR_ETH + 0x2DC */
	volatile uint32 ieee_r_octets_ok;	/* MBAR_ETH + 0x2E0 */

	volatile uint32 RES14[6];		/* MBAR_ETH + 0x2E4-2FC */

	volatile uint32 RES15[64];		/* MBAR_ETH + 0x300-3FF */
} ethernet_regs;

/* Receive & Transmit Buffer Descriptor definitions */
typedef struct BufferDescriptor {
	uint16 status;
	uint16 dataLength;
	uint32 dataPointer;
} FEC_RBD;

typedef struct {
	uint16 status;
	uint16 dataLength;
	uint32 dataPointer;
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
#define FEC_MAX_PKT_SIZE	1536

typedef struct {
	uint8 frame[FEC_MAX_PKT_SIZE];
} mpc512x_frame;

typedef struct {
	FEC_RBD rbd[FEC_RBD_NUM];			/* RBD ring */
	FEC_TBD tbd[FEC_TBD_NUM];			/* TBD ring */
	mpc512x_frame recv_frames[FEC_RBD_NUM];		/* receive buff */
} mpc512x_buff_descs;

typedef struct {
	ethernet_regs *eth;
	xceiver_type xcv_type;		/* transceiver type */
	mpc512x_buff_descs *bdBase;	/* BD rings and recv buffer */
	uint16 rbdIndex;		/* next receive BD to read */
	uint16 tbdIndex;		/* next transmit BD to send */
	uint16 usedTbdIndex;		/* next transmit BD to clean */
	uint16 cleanTbdNum;		/* the number of available transmit BDs */
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

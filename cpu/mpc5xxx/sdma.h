/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on code
 * (C) Copyright Motorola, Inc., 2000
 *
 * odin smartdma header file
 */

#ifndef __MPC5XXX_SDMA_H
#define __MPC5XXX_SDMA_H

#include <common.h>
#include <mpc5xxx.h>

/* Task number assignment */
#define FEC_RECV_TASK_NO            0
#define FEC_XMIT_TASK_NO            1

/*---------------------------------------------------------------------*/

/* Stuff for Ethernet Tx/Rx tasks                                      */

/*---------------------------------------------------------------------*/

/* Layout of Ethernet controller Parameter SRAM area:
----------------------------------------------------------------
0x00: TBD_BASE, base address of TX BD ring
0x04: TBD_NEXT, address of next TX BD to be processed
0x08: RBD_BASE, base address of RX BD ring
0x0C: RBD_NEXT, address of next RX BD to be processed
---------------------------------------------------------------
ALL PARAMETERS ARE ALL LONGWORDS (FOUR BYTES EACH).
*/

/* base address of SRAM area to store parameters used by Ethernet tasks */
#define FEC_PARAM_BASE		(MPC5XXX_SRAM + 0x0800)

/* base address of SRAM area for buffer descriptors */
#define FEC_BD_BASE		(MPC5XXX_SRAM + 0x0820)

/*---------------------------------------------------------------------*/

/* common shortcuts  used  by driver C code                            */

/*---------------------------------------------------------------------*/

/* Disable SmartDMA task */
#define SDMA_TASK_DISABLE(tasknum)                     \
{                                                      \
    volatile ushort *tcr = (ushort *)(MPC5XXX_SDMA + 0x0000001c + 2 * tasknum); \
    *tcr = (*tcr) & (~0x8000);                         \
}

/* Enable SmartDMA task */
#define SDMA_TASK_ENABLE(tasknum)                      \
{                                                      \
    volatile ushort *tcr = (ushort *) (MPC5XXX_SDMA + 0x0000001c + 2 * tasknum); \
    *tcr = (*tcr)  | 0x8000;                           \
}

/* Enable interrupt */
#define SDMA_INT_ENABLE(tasknum)                       \
{                                                      \
    struct mpc5xxx_sdma *sdma = (struct mpc5xxx_sdma *)MPC5XXX_SDMA; \
    sdma->IntMask &= ~(1 << tasknum);                  \
}

/* Disable interrupt */
#define SDMA_INT_DISABLE(tasknum)   \
{                                                      \
    struct mpc5xxx_sdma *sdma = (struct mpc5xxx_sdma *)MPC5XXX_SDMA; \
    sdma->IntMask |= (1 << tasknum);                   \
}


/* Clear interrupt pending bits */
#define SDMA_CLEAR_IEVENT(tasknum)  \
{                                                      \
    struct mpc5xxx_sdma *sdma = (struct mpc5xxx_sdma *)MPC5XXX_SDMA; \
    sdma->IntPend = (1 << tasknum);                    \
}

/* get interupt pending bit of a task */
#define SDMA_GET_PENDINGBIT(tasknum)                   \
	((*(vu_long *)(MPC5XXX_SDMA + 0x14)) & (1<<(tasknum)))

/* get interupt mask bit of a task */
#define SDMA_GET_MASKBIT(tasknum)                      \
	((*(vu_long *)(MPC5XXX_SDMA + 0x18)) & (1<<(tasknum)))

#endif	/* __MPC5XXX_SDMA_H */

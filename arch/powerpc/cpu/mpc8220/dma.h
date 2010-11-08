/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This file is based on code
 * (C) Copyright Motorola, Inc., 2000
 *
 * MPC8220 dma header file
 */

#ifndef __MPC8220_DMA_H
#define __MPC8220_DMA_H

#include <common.h>
#include <mpc8220.h>

/* Task number assignment */
#define FEC_RECV_TASK_NO            0
#define FEC_XMIT_TASK_NO            1

/*---------------------------------------------------------------------
 * Stuff for Ethernet Tx/Rx tasks
 *---------------------------------------------------------------------
 */

/* Layout of Ethernet controller Parameter SRAM area:
 * ----------------------------------------------------------------
 * 0x00: TBD_BASE, base address of TX BD ring
 * 0x04: TBD_NEXT, address of next TX BD to be processed
 * 0x08: RBD_BASE, base address of RX BD ring
 * 0x0C: RBD_NEXT, address of next RX BD to be processed
 * ---------------------------------------------------------------
 * ALL PARAMETERS ARE ALL LONGWORDS (FOUR BYTES EACH).
 */

/* base address of SRAM area to store parameters used by Ethernet tasks */
#define FEC_PARAM_BASE  (MMAP_SRAM + 0x5b00)

/* base address of SRAM area for buffer descriptors */
#define FEC_BD_BASE     (MMAP_SRAM + 0x5b20)

/*---------------------------------------------------------------------
 * common shortcuts  used  by driver C code
 *---------------------------------------------------------------------
 */

/* Disable SmartDMA task */
#define DMA_TASK_DISABLE(tasknum)						\
{										\
	volatile ushort *tcr = (ushort *)(MMAP_DMA + 0x0000001c + 2 * tasknum); \
	*tcr = (*tcr) & (~0x8000);						\
}

/* Enable SmartDMA task */
#define DMA_TASK_ENABLE(tasknum)						\
{										\
	volatile ushort *tcr = (ushort *) (MMAP_DMA + 0x0000001c + 2 * tasknum);\
	*tcr = (*tcr)  | 0x8000;						\
}

/* Clear interrupt pending bits */
#define DMA_CLEAR_IEVENT(tasknum)						\
{										\
	struct mpc8220_dma *dma = (struct mpc8220_dma *)MMAP_DMA;		\
	dma->IntPend = (1 << tasknum);						\
}

#endif  /* __MPC8220_DMA_H */

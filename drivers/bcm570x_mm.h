
/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2000 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/* This program is free software; you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, located in the file LICENSE.                 */
/*                                                                            */
/******************************************************************************/

#ifndef MM_H
#define MM_H

#define __raw_readl readl
#define __raw_writel writel

#define BIG_ENDIAN_HOST 1
#define readl(addr) (*(volatile unsigned int*)(addr))
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))

/* Define memory barrier function here if needed */
#define wmb()
#define membar()
#include <common.h>
#include <asm/types.h>
#include "bcm570x_lm.h"
#include "bcm570x_queue.h"
#include "tigon3.h"
#include <pci.h>

#define FALSE 0
#define TRUE  1
#define ERROR -1

#if DBG
#define STATIC
#else
#define STATIC static
#endif

extern int MM_Packet_Desc_Size;

#define MM_PACKET_DESC_SIZE MM_Packet_Desc_Size

DECLARE_QUEUE_TYPE(UM_RX_PACKET_Q, MAX_RX_PACKET_DESC_COUNT+1);

#define MAX_MEM 16

/* Synch */
typedef int mutex_t;
typedef int spinlock_t;

/* Embedded device control */
typedef struct _UM_DEVICE_BLOCK {
	LM_DEVICE_BLOCK lm_dev;
	pci_dev_t pdev;
	char *name;
	void *mem_list[MAX_MEM];
	dma_addr_t dma_list[MAX_MEM];
	int mem_size_list[MAX_MEM];
	int mem_list_num;
	int mtu;
	int index;
	int opened;
	int delayed_link_ind; /* Delay link status during initial load */
	int adapter_just_inited; /* the first few seconds after init. */
	int spurious_int;            /* new -- unsupported */
	int timer_interval;
	int adaptive_expiry;
	int crc_counter_expiry;         /* new -- unsupported */
	int poll_tib_expiry;         /* new -- unsupported */
	int tx_full;
	int tx_queued;
	int line_speed;		/* in Mbps, 0 if link is down */
	UM_RX_PACKET_Q rx_out_of_buf_q;
	int rx_out_of_buf;
	int rx_low_buf_thresh; /* changed to rx_buf_repl_thresh */
	int rx_buf_repl_panic_thresh;
	int rx_buf_align;            /* new -- unsupported */
	int do_global_lock;
	mutex_t global_lock;
	mutex_t undi_lock;
	long undi_flags;
	volatile int interrupt;
	int tasklet_pending;
	int tasklet_busy;	     /* new -- unsupported */
	int rx_pkt;
	int tx_pkt;
#ifdef NICE_SUPPORT   /* unsupported, this is a linux ioctl */
	void (*nice_rx)(void*, void* );
	void* nice_ctx;
#endif /* NICE_SUPPORT */
	int rx_adaptive_coalesce;
	unsigned int rx_last_cnt;
	unsigned int tx_last_cnt;
	unsigned int rx_curr_coalesce_frames;
	unsigned int rx_curr_coalesce_ticks;
	unsigned int tx_curr_coalesce_frames;  /* new -- unsupported */
#if TIGON3_DEBUG          /* new -- unsupported */
	uint tx_zc_count;
	uint tx_chksum_count;
	uint tx_himem_count;
	uint rx_good_chksum_count;
#endif
	unsigned int rx_bad_chksum_count;   /* new -- unsupported */
	unsigned int rx_misc_errors;        /* new -- unsupported */
} UM_DEVICE_BLOCK, *PUM_DEVICE_BLOCK;


/* Physical/PCI DMA address */
typedef union {
	dma_addr_t dma_map;
} dma_map_t;

/* Packet */
typedef struct
_UM_PACKET {
    LM_PACKET lm_packet;
    void* skbuff;      /* Address of packet buffer */
} UM_PACKET, *PUM_PACKET;

#define MM_ACQUIRE_UNDI_LOCK(_pDevice)
#define MM_RELEASE_UNDI_LOCK(_pDevice)
#define MM_ACQUIRE_INT_LOCK(_pDevice)
#define MM_RELEASE_INT_LOCK(_pDevice)
#define MM_UINT_PTR(_ptr)   ((unsigned long) (_ptr))

/* Macro for setting 64bit address struct */
#define set_64bit_addr(paddr, low, high) \
	(paddr)->Low = low;             \
	(paddr)->High = high;

/* Assume that PCI controller's view of host memory is same as host */

#define MEM_TO_PCI_PHYS(addr) (addr)

extern void MM_SetAddr (LM_PHYSICAL_ADDRESS *paddr, dma_addr_t addr);
extern void MM_SetT3Addr(T3_64BIT_HOST_ADDR *paddr, dma_addr_t addr);
extern void MM_MapTxDma (PLM_DEVICE_BLOCK pDevice,
			 struct _LM_PACKET *pPacket, T3_64BIT_HOST_ADDR *paddr,
			 LM_UINT32 *len, int frag);
extern void MM_MapRxDma ( PLM_DEVICE_BLOCK pDevice,
			  struct _LM_PACKET *pPacket,
			  T3_64BIT_HOST_ADDR *paddr);


/* BSP needs to provide sysUsecDelay and sysSerialPrintString */
extern void sysSerialPrintString (char *s);
#define MM_Wait(usec) udelay(usec)

/* Define memory barrier function here if needed */
#define wmb()

#if 0
#define cpu_to_le32(val) LONGSWAP(val)
#endif
#endif /* MM_H */

/*
 * dma.h - Blackfin DMA defines/structures/etc...
 *
 * Copyright 2004-2008 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#ifndef _BLACKFIN_DMA_H_
#define _BLACKFIN_DMA_H_

#include <linux/types.h>
#ifdef __ADSPBF60x__
#include <asm/mach-common/bits/dde.h>
#else
#include <asm/mach-common/bits/dma.h>
#endif

struct dmasg_large {
	void *next_desc_addr;
	u32 start_addr;
	u16 cfg;
	u16 x_count;
	s16 x_modify;
	u16 y_count;
	s16 y_modify;
} __attribute__((packed));

struct dmasg {
	u32 start_addr;
	u16 cfg;
	u16 x_count;
	s16 x_modify;
	u16 y_count;
	s16 y_modify;
} __attribute__((packed));

struct dma_register {
#ifdef __ADSPBF60x__
	void *next_desc_ptr;	/* DMA Next Descriptor Pointer register */
	u32 start_addr;		/* DMA Start address  register */
	u32 config;		/* DMA Configuration register */

	u32 x_count;		/* DMA x_count register */
	s32 x_modify;		/* DMA x_modify register */
	u32 y_count;		/* DMA y_count register */
	s32 y_modify;		/* DMA y_modify register */
	u32 __pad0[2];

	void *curr_desc_ptr;	/* DMA Curr Descriptor Pointer register */
	void *prev_desc_ptr;	/* DMA Prev Descriptor Pointer register */
	void *curr_addr;	/* DMA Current Address Pointer register */
	u32 status;		/* DMA irq status register */
	u32 curr_x_count;	/* DMA Current x-count register */
	u32 curr_y_count;	/* DMA Current y-count register */
	u32 __pad1[2];

	u32 bw_limit;		/* DMA Bandwidth Limit Count */
	u32 curr_bw_limit;	/* DMA curr Bandwidth Limit Count */
	u32 bw_monitor;		/* DMA Bandwidth Monitor Count */
	u32 curr_bw_monitor;	/* DMA curr Bandwidth Monitor Count */
#else
	void *next_desc_ptr;	/* DMA Next Descriptor Pointer register */
	u32 start_addr;		/* DMA Start address  register */

	u16 config;		/* DMA Configuration register */
	u16 dummy1;		/* DMA Configuration register */

	u32 reserved;

	u16 x_count;		/* DMA x_count register */
	u16 dummy2;

	s16 x_modify;		/* DMA x_modify register */
	u16 dummy3;

	u16 y_count;		/* DMA y_count register */
	u16 dummy4;

	s16 y_modify;		/* DMA y_modify register */
	u16 dummy5;

	void *curr_desc_ptr;	/* DMA Current Descriptor Pointer register */

	u32 curr_addr_ptr;	/* DMA Current Address Pointer register */

	u16 status;		/* DMA irq status register */
	u16 dummy6;

	u16 peripheral_map;	/* DMA peripheral map register */
	u16 dummy7;

	u16 curr_x_count;	/* DMA Current x-count register */
	u16 dummy8;

	u32 reserved2;

	u16 curr_y_count;	/* DMA Current y-count register */
	u16 dummy9;

	u32 reserved3;
#endif
};

#endif

/*
 * Blackfin MUSB HCD (Host Controller Driver) for u-boot
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BLACKFIN_USB_H__
#define __BLACKFIN_USB_H__

#include <linux/types.h>

/* Every register is 32bit aligned, but only 16bits in size */
#define ureg(name) u16 name; u16 __pad_##name;

#define musb_regs musb_regs
struct musb_regs {
	/* common registers */
	ureg(faddr)
	ureg(power)
	ureg(intrtx)
	ureg(intrrx)
	ureg(intrtxe)
	ureg(intrrxe)
	ureg(intrusb)
	ureg(intrusbe)
	ureg(frame)
	ureg(index)
	ureg(testmode)
	ureg(globintr)
	ureg(global_ctl)
	u32	reserved0[3];
	/* indexed registers */
	ureg(txmaxp)
	ureg(txcsr)
	ureg(rxmaxp)
	ureg(rxcsr)
	ureg(rxcount)
	ureg(txtype)
	ureg(txinterval)
	ureg(rxtype)
	ureg(rxinterval)
	u32	reserved1;
	ureg(txcount)
	u32	reserved2[5];
	/* fifo */
	u16	fifox[32];
	/* OTG, dynamic FIFO, version & vendor registers */
	u32	reserved3[16];
	ureg(devctl)
	ureg(vbus_irq)
	ureg(vbus_mask)
	u32 reserved4[15];
	ureg(linkinfo)
	ureg(vplen)
	ureg(hseof1)
	ureg(fseof1)
	ureg(lseof1)
	u32 reserved5[41];
	/* target address registers */
	struct musb_tar_regs {
		ureg(txmaxp)
		ureg(txcsr)
		ureg(rxmaxp)
		ureg(rxcsr)
		ureg(rxcount)
		ureg(txtype)
		ureg(txinternal)
		ureg(rxtype)
		ureg(rxinternal)
		u32	reserved6;
		ureg(txcount)
		u32 reserved7[5];
	} tar[8];
} __attribute__((packed));

struct bfin_musb_dma_regs {
	ureg(interrupt);
	ureg(control);
	ureg(addr_low);
	ureg(addr_high);
	ureg(count_low);
	ureg(count_high);
	u32 reserved0[2];
};

#undef ureg

/* EP5-EP7 are the only ones with 1024 byte FIFOs which BULK really needs */
#define MUSB_BULK_EP 5

/* Blackfin FIFO's are static */
#define MUSB_NO_DYNAMIC_FIFO

/* No HUB support :( */
#define MUSB_NO_MULTIPOINT

#endif

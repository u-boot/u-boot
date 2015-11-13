/* GRLIB Memory controller setup structures
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MEMCFG_H__
#define __MEMCFG_H__

/*********** Low Level Memory Controller Initalization ***********/

#ifndef __ASSEMBLER__

struct grlib_mctrl_handler;

typedef void (*mctrl_handler_t)(
	struct grlib_mctrl_handler *dev,
	void *conf,
	unsigned int ioarea
	);

/* Memory Controller Handler Structure */
struct grlib_mctrl_handler {
	unsigned char	type;		/* 0x00. MASK: AHB MST&SLV, APB SLV */
	char		index;		/* 0x01. Unit number, 0, 1, 2... */
	char		unused[2];	/* 0x02 */
	unsigned int	ven_dev;	/* 0x04. Device and Vendor */
	mctrl_handler_t	func;		/* 0x08. Memory Controller Handler */
	void		*priv;		/* 0x0c. Optional private data, ptr to
					 * info how to set up controller */
};

extern struct grlib_mctrl_handler grlib_mctrl_handlers[];

#endif

#define MH_STRUCT_SIZE		(4*4)
#define MH_TYPE			0x00
#define MH_INDEX		0x01
#define MH_VENDOR_DEVICE	0x04
#define MH_FUNC			0x08
#define MH_PRIV			0x0c

#define MH_TYPE_NONE	DEV_NONE
#define MH_TYPE_AHB_MST	DEV_AHB_MST
#define MH_TYPE_AHB_SLV	DEV_AHB_SLV
#define MH_TYPE_APB_SLV	DEV_APB_SLV

#define MH_UNUSED	{0, 0}
#define MH_END		{DEV_NONE, 0, MH_UNUSED, AMBA_PNP_ID(0, 0), 0, 0}

/*********** Low Level Memory Controller Initalization Handlers ***********/

#ifndef __ASSEMBLER__
extern void _nomem_mctrl_init(
	struct grlib_mctrl_handler *dev,
	void *conf,
	unsigned int ioarea_apbmst);

struct mctrl_setup {
	unsigned int reg_mask;		/* Which registers to write */
	struct {
		unsigned int mask;	/* Mask used keep reg bits unchanged */
		unsigned int value;	/* Value written to register */
	} regs[8];
};

extern void _nomem_ahbmctrl_init(
	struct grlib_mctrl_handler *dev,
	void *conf,
	unsigned int ioarea_apbmst);

struct ahbmctrl_setup {
	int ahb_mbar_no;		/* MBAR to get register address from */
	unsigned int reg_mask;		/* Which registers to write */
	struct {
		unsigned int mask;	/* Mask used keep reg bits unchanged */
		unsigned int value;	/* Value written to register */
	} regs[8];
};
#endif

/* mctrl_setup data structure defines */
#define NREGS_OFS 0
#define REGS_OFS 0x4
#define REGS_SIZE 8

#endif

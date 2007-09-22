/* (C) Copyright 2007 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __IMMAP_85xx_fsl_pci__
#define __IMMAP_85xx_fsl_pci__

/*
 * Common PCI/PCIE Register structure for mpc85xx and mpc86xx
 */

/*
 * PCI Translation Registers
 */
typedef struct pci_outbound_window {
	u32	potar;		/* 0x00 - Address */
	u32	potear;		/* 0x04 - Address Extended */
	u32	powbar;		/* 0x08 - Window Base Address */
	u32	res1;
	u32	powar;		/* 0x10 - Window Attributes */
#define POWAR_EN	0x80000000
#define POWAR_IO_READ	0x00080000
#define POWAR_MEM_READ	0x00040000
#define POWAR_IO_WRITE	0x00008000
#define POWAR_MEM_WRITE	0x00004000
	u32	res2[3];
} pot_t;

typedef struct pci_inbound_window {
	u32	pitar;		/* 0x00 - Address */
	u32	res1;
	u32	piwbar;		/* 0x08 - Window Base Address */
	u32	piwbear;	/* 0x0c - Window Base Address Extended */
	u32	piwar;		/* 0x10 - Window Attributes */
#define PIWAR_EN		0x80000000
#define PIWAR_PF		0x20000000
#define PIWAR_LOCAL		0x00f00000
#define PIWAR_READ_SNOOP	0x00050000
#define PIWAR_WRITE_SNOOP	0x00005000
	u32	res2[3];
} pit_t;

/* PCI/PCI Express Registers */
typedef struct ccsr_pci {
	u32	cfg_addr;	/* 0x000 - PCI Configuration Address Register */
	u32	cfg_data;	/* 0x004 - PCI Configuration Data Register */
	u32	int_ack;	/* 0x008 - PCI Interrupt Acknowledge Register */
	u32	out_comp_to;	/* 0x00C - PCI Outbound Completion Timeout Register */
	u32	out_conf_to;	/* 0x010 - PCI Configuration Timeout Register */
	u32	config;		/* 0x014 - PCIE CONFIG Register */
	char	res2[8];
	u32	pme_msg_det;	/* 0x020 - PCIE PME & message detect register */
	u32	pme_msg_dis;	/* 0x024 - PCIE PME & message disable register */
	u32	pme_msg_int_en;	/* 0x028 - PCIE PME & message interrupt enable register */
	u32	pm_command;	/* 0x02c - PCIE PM Command register */
	char	res4[3016];	/*     (- #xbf8	 #x30)3016 */
	u32	block_rev1;	/* 0xbf8 - PCIE Block Revision register 1 */
	u32	block_rev2;	/* 0xbfc - PCIE Block Revision register 2 */

	pot_t	pot[5];		/* 0xc00 - 0xc9f Outbound ATMU's 0, 1, 2, 3, and 4 */
	u32	res5[64];
	pit_t	pit[3];		/* 0xda0 - 0xdff Inbound ATMU's 3, 2, and 1 */
#define PIT3 0
#define PIT2 1
#define PIT1 2

#if 0
	u32	potar0;		/* 0xc00 - PCI Outbound Transaction Address Register 0 */
	u32	potear0;	/* 0xc04 - PCI Outbound Translation Extended Address Register 0 */
	char	res5[8];
	u32	powar0;		/* 0xc10 - PCI Outbound Window Attributes Register 0 */
	char	res6[12];
	u32	potar1;		/* 0xc20 - PCI Outbound Transaction Address Register 1 */
	u32	potear1;	/* 0xc24 - PCI Outbound Translation Extended Address Register 1 */
	u32	powbar1;	/* 0xc28 - PCI Outbound Window Base Address Register 1 */
	char	res7[4];
	u32	powar1;		/* 0xc30 - PCI Outbound Window Attributes Register 1 */
	char	res8[12];
	u32	potar2;		/* 0xc40 - PCI Outbound Transaction Address Register 2 */
	u32	potear2;	/* 0xc44 - PCI Outbound Translation Extended Address Register 2 */
	u32	powbar2;	/* 0xc48 - PCI Outbound Window Base Address Register 2 */
	char	res9[4];
	u32	powar2;		/* 0xc50 - PCI Outbound Window Attributes Register 2 */
	char	res10[12];
	u32	potar3;		/* 0xc60 - PCI Outbound Transaction Address Register 3 */
	u32	potear3;	/* 0xc64 - PCI Outbound Translation Extended Address Register 3 */
	u32	powbar3;	/* 0xc68 - PCI Outbound Window Base Address Register 3 */
	char	res11[4];
	u32	powar3;		/* 0xc70 - PCI Outbound Window Attributes Register 3 */
	char	res12[12];
	u32	potar4;		/* 0xc80 - PCI Outbound Transaction Address Register 4 */
	u32	potear4;	/* 0xc84 - PCI Outbound Translation Extended Address Register 4 */
	u32	powbar4;	/* 0xc88 - PCI Outbound Window Base Address Register 4 */
	char	res13[4];
	u32	powar4;		/* 0xc90 - PCI Outbound Window Attributes Register 4 */
	char	res14[268];
	u32	pitar3;		/* 0xda0 - PCI Inbound Translation Address Register 3 */
	char	res15[4];
	u32	piwbar3;	/* 0xda8 - PCI Inbound Window Base Address Register 3 */
	u32	piwbear3;	/* 0xdac - PCI Inbound Window Base Extended Address Register 3 */
	u32	piwar3;		/* 0xdb0 - PCI Inbound Window Attributes Register 3 */
	char	res16[12];
	u32	pitar2;		/* 0xdc0 - PCI Inbound Translation Address Register 2 */
	char	res17[4];
	u32	piwbar2;	/* 0xdc8 - PCI Inbound Window Base Address Register 2 */
	u32	piwbear2;	/* 0xdcc - PCI Inbound Window Base Extended Address Register 2 */
	u32	piwar2;		/* 0xdd0 - PCI Inbound Window Attributes Register 2 */
	char	res18[12];
	u32	pitar1;		/* 0xde0 - PCI Inbound Translation Address Register 1 */
	char	res19[4];
	u32	piwbar1;	/* 0xde8 - PCI Inbound Window Base Address Register 1 */
	char	res20[4];
	u32	piwar1;		/* 0xdf0 - PCI Inbound Window Attributes Register 1 */
	char	res21[12];
#endif
	u32	pedr;		/* 0xe00 - PCI Error Detect Register */
	u32	pecdr;		/* 0xe04 - PCI Error Capture Disable Register */
	u32	peer;		/* 0xe08 - PCI Error Interrupt Enable Register */
	u32	peattrcr;	/* 0xe0c - PCI Error Attributes Capture Register */
	u32	peaddrcr;	/* 0xe10 - PCI Error Address Capture Register */
/*	u32	perr_disr	 * 0xe10 - PCIE Erorr Disable Register */
	u32	peextaddrcr;	/* 0xe14 - PCI	Error Extended Address Capture Register */
	u32	pedlcr;		/* 0xe18 - PCI Error Data Low Capture Register */
	u32	pedhcr;		/* 0xe1c - PCI Error Error Data High Capture Register */
	u32	gas_timr;	/* 0xe20 - PCI Gasket Timer Register */
/*	u32	perr_cap_stat;	 * 0xe20 - PCIE Error Capture Status Register */
	char	res22[4];
	u32	perr_cap0;	/* 0xe28 - PCIE Error Capture Register 0 */
	u32	perr_cap1;	/* 0xe2c - PCIE Error Capture Register 1 */
	u32	perr_cap2;	/* 0xe30 - PCIE Error Capture Register 2 */
	u32	perr_cap3;	/* 0xe34 - PCIE Error Capture Register 3 */
	char	res23[456];	/*     (- #x1000 #xe38) 456 */
} ccsr_fsl_pci_t;

#endif /*__IMMAP_fsl_pci__*/

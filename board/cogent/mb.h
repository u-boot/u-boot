/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * defines for Cogent Motherboards
 */

#ifndef _COGENT_MB_H
#define _COGENT_MB_H

/*
 * Cogent Motherboard Address Map
 *
 * The size of a Cogent motherboard address space is 256 Mbytes (i.e. 28 bits).
 *
 * The first 32 Mbyte (0x0000000-0x1FFFFFF) is usually RAM. The following
 * 3 x 32 Mbyte areas (0x2000000-0x3FFFFFF, 0x4000000-0x5FFFFFF and
 * 0x6000000-0x7FFFFFF) are general I/O "slots" (slots 1, 2 and 3).
 * Most other motherboard devices have registers mapped into the area
 * 0xE000000-0xFFFFFFF (Motherboard I/O slot?). The area 0x8000000-0xDFFFFFF
 * is free for whatever.
 *
 * The location of the motherboard address space in the physical address space
 * of the cpu is given by CMA_MB_BASE. This value is determined by the cpu
 * module plugged into the motherboard and is configured above.
 *
 * Motherboard I/O devices mapped into the area (0xE000000-0xFFFFFFF)
 * generally only use byte lane 0 (D0-7) for their transfers, i.e. only
 * 8 bit, or 1 byte, transfers can take place, so all the registers are
 * only 8 bits wide. The exceptions are the motherboard flash, which uses
 * byte lanes 0 and 1 (i.e. 16 bits), and the mapped PCI address space.
 *
 * I/O registers within the mapped motherboard devices are 64 bit aligned
 * i.e. they are 8 bytes apart. For big endian addressing, the 8 bit register
 * will be at byte 7 (the address + 7). For little endian addressing, the
 * register will be at byte 0 (the address + 0). To learn the endianess
 * we must include <endian.h>
 *
 * Take the CMA102 and CMA111 motherboards as examples...
 *
 * The CMA102 has three CMABus I/O Expansion slots and no PCI bridge. The 3
 * CMABus slots are each mapped directly onto the three general I/O slots.
 *
 * The CMA111 has only one CMABus I/O Expansion slot, but has a V360EPC PCI
 * bridge. The CMABus slot is mapped onto general I/O slot 1. The standard
 * PCI Bus space is mapped onto general I/O slot 2, with a small area at the
 * top reserved for access to the V360EPC registers (0x5FF0000-0x5FFFFFF).
 * I/O slot 3 is unused. The extended PCI Bus space is mapped onto the area
 * 0xA000000-0xDFFFFFF.
 */

#define CMA_MB_RAM_BASE		(CFG_CMA_MB_BASE+0x0000000)
#define CMA_MB_RAM_SIZE		0x2000000	/* dip sws set actual size */

#if (CMA_MB_CAPS & CMA_MB_CAP_SLOT1)
#define CMA_MB_SLOT1_BASE	(CFG_CMA_MB_BASE+0x2000000)
#define CMA_MB_SLOT1_SIZE	0x2000000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SLOT2)
#define CMA_MB_SLOT2_BASE	(CFG_CMA_MB_BASE+0x4000000)
#define CMA_MB_SLOT2_SIZE	0x2000000
#endif
#if (CMA_MB_CAPS & CMA_MB_CAP_PCI)
#define CMA_MB_STDPCI_BASE	(CFG_CMA_MB_BASE+0x4000000)
#define CMA_MB_STDPCI_SIZE	0x1ff0000
#define CMA_MB_V360EPC_BASE	(CFG_CMA_MB_BASE+0x5ff0000)
#define CMA_MB_V360EPC_SIZE	0x10000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SLOT3)
#define CMA_MB_SLOT3_BASE	(CFG_CMA_MB_BASE+0x6000000)
#define CMA_MB_SLOT3_SIZE	0x2000000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI_EXT)
#define CMA_MB_EXTPCI_BASE	(CFG_CMA_MB_BASE+0xa000000)
#define CMA_MB_EXTPCI_SIZE	0x4000000
#endif

#define CMA_MB_ROMLOW_BASE	(CFG_CMA_MB_BASE+0xe000000)
#define CMA_MB_ROMLOW_SIZE	0x800000
#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
#define CMA_MB_FLLOW_EXEC_BASE	(CFG_CMA_MB_BASE+0xe000000)
#define CMA_MB_FLLOW_EXEC_SIZE	0x100000
#define CMA_MB_FLLOW_RDWR_BASE	(CFG_CMA_MB_BASE+0xe400000)
#define CMA_MB_FLLOW_RDWR_SIZE	0x400000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_RTC)
#define CMA_MB_RTC_BASE		(CFG_CMA_MB_BASE+0xe800000)
#define CMA_MB_RTC_SIZE		0x4000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SERPAR)
#define CMA_MB_SERPAR_BASE	(CFG_CMA_MB_BASE+0xe900000)
#define   CMA_MB_SERIALB_BASE	  (CMA_MB_SERPAR_BASE+0x00)
#define   CMA_MB_SERIALA_BASE	  (CMA_MB_SERPAR_BASE+0x40)
#define   CMA_MB_PARALLEL_BASE	  (CMA_MB_SERPAR_BASE+0x80)
#define CMA_MB_SERPAR_SIZE	0xa0
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_KBM)
#define CMA_MB_PKBM_BASE	(CFG_CMA_MB_BASE+0xe900100)
#define CMA_MB_PKBM_SIZE	0x10
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_LCD)
#define CMA_MB_LCD_BASE		(CFG_CMA_MB_BASE+0xeb00000)
#define CMA_MB_LCD_SIZE		0x10
#endif

#define CMA_MB_DIPSW_BASE	(CFG_CMA_MB_BASE+0xec00000)
#define CMA_MB_DIPSW_SIZE	0x10

#if (CMA_MB_CAPS & (CMA_MB_CAP_SLOT1|CMA_MB_CAP_SER2|CMA_MB_CAP_KBM))
#define CMA_MB_SLOT1CFG_BASE	(CFG_CMA_MB_BASE+0xf100000)
#if (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define   CMA_MB_SER2_BASE	  (CMA_MB_SLOT1CFG_BASE+0x80)
#define     CMA_MB_SER2B_BASE	    (CMA_MB_SER2_BASE+0x00)
#define     CMA_MB_SER2A_BASE	    (CMA_MB_SER2_BASE+0x40)
#endif
#if defined(CONFIG_CMA302) && defined(CONFIG_CMA302_SLOT1)
#define   CMA_MB_S1KBM_BASE	  (CMA_MB_SLOT1CFG_BASE+0x200)
#endif
#if (CMA_MB_CAPS & CMA_MB_CAP_KBM) && !defined(COGENT_CMA150)
#define   CMA_MB_IREQ1STAT_BASE	  (CMA_MB_SLOT1CFG_BASE+0x100)
#define   CMA_MB_AKBM_BASE	  (CMA_MB_SLOT1CFG_BASE+0x200)
#define   CMA_MB_IREQ1MASK_BASE	  (CMA_MB_SLOT1CFG_BASE+0x300)
#endif
#define CMA_MB_SLOT1CFG_SIZE	0x400
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SLOT2)
#define CMA_MB_SLOT2CFG_BASE	(CFG_CMA_MB_BASE+0xf200000)
#if defined(CONFIG_CMA302) && defined(CONFIG_CMA302_SLOT2)
#define   CMA_MB_S2KBM_BASE	  (CMA_MB_SLOT2CFG_BASE+0x200)
#endif
#define CMA_MB_SLOT2CFG_SIZE	0x400
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI)
#define CMA_MB_PCICTL_BASE	(CFG_CMA_MB_BASE+0xf200000)
#define   CMA_MB_PCI_V3CTL_BASE	  (CMA_MB_PCICTL_BASE+0x100)
#define   CMA_MB_PCI_IDSEL_BASE	  (CMA_MB_PCICTL_BASE+0x200)
#define   CMA_MB_PCI_IMASK_BASE	  (CMA_MB_PCICTL_BASE+0x300)
#define   CMA_MB_PCI_ISTAT_BASE	  (CMA_MB_PCICTL_BASE+0x400)
#define   CMA_MB_PCI_MBID_BASE	  (CMA_MB_PCICTL_BASE+0x500)
#define   CMA_MB_PCI_MBREV_BASE	  (CMA_MB_PCICTL_BASE+0x600)
#define CMA_MB_PCICTL_SIZE	0x700
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SLOT3)
#define CMA_MB_SLOT3CFG_BASE	(CFG_CMA_MB_BASE+0xf300000)
#if defined(CONFIG_CMA302) && defined(CONFIG_CMA302_SLOT3)
#define   CMA_MB_S3KBM_BASE	  (CMA_MB_SLOT3CFG_BASE+0x200)
#endif
#define CMA_MB_SLOT3CFG_SIZE	0x400
#endif

#define CMA_MB_ROMHIGH_BASE	(CFG_CMA_MB_BASE+0xf800000)
#define CMA_MB_ROMHIGH_SIZE	0x800000
#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
#define CMA_MB_FLHIGH_EXEC_BASE	(CFG_CMA_MB_BASE+0xf800000)
#define CMA_MB_FLHIGH_EXEC_SIZE	0x100000
#define CMA_MB_FLHIGH_RDWR_BASE	(CFG_CMA_MB_BASE+0xfc00000)
#define CMA_MB_FLHIGH_RDWR_SIZE	0x400000
#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI)

/* PCI Control Register bits */

/* V360EPC Control register bits */
#define CMA_MB_PCI_V3CTL_RESET	0x01
#define CMA_MB_PCI_V3CTL_EXTADD	0x08

/* PCI ID Select register bits */
#define CMA_MB_PCI_IDSEL_SLOTA	0x01
#define CMA_MB_PCI_IDSEL_SLOTB	0x02
#define CMA_MB_PCI_IDSEL_GD82559 0x04
#define CMA_MB_PCI_IDSEL_B69000	0x08
#define CMA_MB_PCI_IDSEL_PD6832	0x10

/* PCI Interrupt Mask/Status register bits */
#define CMA_MB_PCI_IMS_INTA	0x01
#define CMA_MB_PCI_IMS_INTB	0x02
#define CMA_MB_PCI_IMS_INTC	0x04
#define CMA_MB_PCI_IMS_INTD	0x08
#define CMA_MB_PCI_IMS_CBINT	0x10
#define CMA_MB_PCI_IMS_V3LINT	0x80

#endif

#if (CMA_MB_CAPS & (CMA_MB_CAP_KBM|CMA_MB_CAP_SER2)) && !defined(COGENT_CMA150)

/*
 * IREQ1 Interrupt Mask/Status register bits
 * (Note: not available on CMA150 - must poll HT6542B interrupt register)
 */

#define IREQ1_MINT	0x01
#define IREQ1_KINT	0x02
#if (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define IREQ1_SINT2	0x04
#define IREQ1_SINT3	0x08
#endif

#endif

#ifndef __ASSEMBLY__

#ifdef USE_HOSTCC
#include <endian.h>		/* avoid using private kernel header files */
#else
#include <asm/byteorder.h>	/* use U-Boot provided headers */
#endif

/* a single CMA10x motherboard i/o register */
typedef
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned char value;
#endif
	unsigned char filler[7];
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned char value;
#endif
    }
cma_mb_reg;

extern __inline__ unsigned char
cma_mb_reg_read(volatile cma_mb_reg *reg)
{
    unsigned char data = reg->value;
    __asm__ __volatile__ ("eieio" : : : "memory");
    return data;
}

extern __inline__ void
cma_mb_reg_write(volatile cma_mb_reg *reg, unsigned char data)
{
    reg->value = data;
    __asm__ __volatile__ ("eieio" : : : "memory");
}

#if (CMA_MB_CAPS & CMA_MB_CAP_RTC)

/* MK48T02 RTC registers */
typedef
    struct {
	cma_mb_reg sram[2040];/* Battery-Backed SRAM */
	cma_mb_reg clk_ctl;	/* Clock Control Register */
	cma_mb_reg clk_sec;	/* Clock Seconds Register */
	cma_mb_reg clk_min;	/* Clock Minutes Register */
	cma_mb_reg clk_hour;	/* Clock Hour Register */
	cma_mb_reg clk_day;	/* Clock Day Register */
	cma_mb_reg clk_date;	/* Clock Date Register */
	cma_mb_reg clk_month;	/* Clock Month Register */
	cma_mb_reg clk_year;	/* Clock Year Register */
    }
cma_mb_rtc;

#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_SERPAR)

/* ST16C522 Serial I/O */
typedef
    struct {
	cma_mb_reg ser_rhr;	/* Receive Holding Register (R, DLAB=0) */
	cma_mb_reg ser_ier;	/* Interrupt Enable Register (R/W, DLAB=0) */
	cma_mb_reg ser_isr;	/* Interrupt Status Register (R) */
	cma_mb_reg ser_lcr;	/* Line Control Register (R/W) */
	cma_mb_reg ser_mcr;	/* Modem Control Register (R/W) */
	cma_mb_reg ser_lsr;	/* Line Status Register (R) */
	cma_mb_reg ser_msr;	/* Modem Status Register (R/W) */
	cma_mb_reg ser_spr;	/* Scratch Pad Register (R/W) */
    }
cma_mb_serial;

#define ser_thr	ser_rhr		/* Transmit Holding Register (W, DLAB=0) */
#define ser_brl	ser_rhr		/* Baud Rate Divisor Low Byte (R/W, DLAB=1) */
#define ser_brh	ser_ier		/* Baud Rate Divisor High Byte (R/W, DLAB=1) */
#define ser_fcr	ser_isr		/* FIFO Control Register (W) */
#define ser_nop	ser_lsr		/* No Operation (W) */

/* ST16C522 Parallel I/O */
typedef
    struct {
	cma_mb_reg par_rdr;	/* Port Read Data Register (R) */
	cma_mb_reg par_sr;	/* Status Register (R) */
	cma_mb_reg par_cmd;	/* Command Register (R) */
    }
cma_mb_parallel;

#define par_wdr	par_rdr		/* Port Write Data Register (W) */
#define par_ios	par_sr		/* I/O Select Register (W) */
#define par_ctl	par_cmd		/* Control Register (W) */

#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_KBM) || defined(CONFIG_CMA302)

/* HT6542B PS/2 Keyboard/Mouse Controller */
typedef
    struct {
	cma_mb_reg kbm_rdr;	/* Read Data Register (R) */
	cma_mb_reg kbm_sr;	/* Status Register (R) */
    }
cma_mb_kbm;

#define kbm_wdr	kbm_rdr		/* Write Data Register (W) */
#define kbm_cmd	kbm_sr		/* Command Register (W) */

#endif

#if (CMA_MB_CAPS & CMA_MB_CAP_LCD)

/* HD44780 LCD Display */
typedef
    struct {
	cma_mb_reg lcd_ccr;	/* Current Character Register (R/W) */
	cma_mb_reg lcd_bsr;	/* Busy Status Register (R) */
    }
cma_mb_lcd;

#define lcd_cmd	lcd_bsr		/* Command Register (W) */

#endif

/* 8-Position Configuration Switch */
typedef
    struct {
	cma_mb_reg dip_val;	/* Dip Switch value (R) */
    }
cma_mb_dipsw;

#if (CMA_MB_CAPS & CMA_MB_CAP_PCI)

/* V360EPC PCI Bridge */
typedef
    struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned short v3_pci_vendor;		/* 0x00 */
	unsigned short v3_pci_device;
	unsigned short v3_pci_cmd;		/* 0x04 */
	unsigned short v3_pci_stat;
	unsigned long  v3_pci_cc_rev;		/* 0x08 */
	unsigned long  v3_pci_hdr_cfg;		/* 0x0c */
	unsigned long  v3_pci_io_base;		/* 0x10 */
	unsigned long  v3_pci_base0;		/* 0x14 */
	unsigned long  v3_pci_base1;		/* 0x18 */
	unsigned long  reserved1[4];		/* 0x1c */
	unsigned short v3_pci_sub_vendor;	/* 0x2c */
	unsigned short v3_pci_sub_id;
	unsigned long  v3_pci_rom;		/* 0x30 */
	unsigned long  reserved2[2];		/* 0x34 */
	unsigned long  v3_pci_bparam;		/* 0x3c */
	unsigned long  v3_pci_map0;		/* 0x40 */
	unsigned long  v3_pci_map1;		/* 0x44 */
	unsigned long  v3_pci_int_stat;		/* 0x48 */
	unsigned long  v3_pci_int_cfg;		/* 0x4c */
	unsigned long  reserved3[1];		/* 0x50 */
	unsigned long  v3_lb_base0;		/* 0x54 */
	unsigned long  v3_lb_base1;		/* 0x58 */
	unsigned short reserved4;		/* 0x5c */
	unsigned short v3_lb_map0;
	unsigned short reserved5;		/* 0x60 */
	unsigned short v3_lb_map1;
	unsigned short v3_lb_base2;		/* 0x64 */
	unsigned short v3_lb_map2;
	unsigned long  v3_lb_size;		/* 0x68 */
	unsigned short reserved6;		/* 0x6c */
	unsigned short v3_lb_io_base;
	unsigned short v3_fifo_cfg;		/* 0x70 */
	unsigned short v3_fifo_priority;
	unsigned short v3_fifo_stat;		/* 0x74 */
	unsigned char  v3_lb_istat;
	unsigned char  v3_lb_imask;
	unsigned short v3_system;		/* 0x78 */
	unsigned short v3_lb_cfg;
	unsigned short v3_pci_cfg;		/* 0x7c */
	unsigned short reserved7;
	unsigned long  v3_dma_pci_addr0;	/* 0x80 */
	unsigned long  v3_dma_local_addr0;	/* 0x84 */
	unsigned long  v3_dma_length0:24;	/* 0x88 */
	unsigned long  v3_dma_csr0:8;
	unsigned long  v3_dma_ctlb_adr0;	/* 0x8c */
	unsigned long  v3_dma_pci_addr1;	/* 0x90 */
	unsigned long  v3_dma_local_addr1;	/* 0x94 */
	unsigned long  v3_dma_length1:24;	/* 0x98 */
	unsigned long  v3_dma_csr1:8;
	unsigned long  v3_dma_ctlb_adr1;	/* 0x9c */
	unsigned long  v3_i20_mups[8];		/* 0xa0 */
	unsigned char  v3_mail_data0;		/* 0xc0 */
	unsigned char  v3_mail_data1;
	unsigned char  v3_mail_data2;
	unsigned char  v3_mail_data3;
	unsigned char  v3_mail_data4;		/* 0xc4 */
	unsigned char  v3_mail_data5;
	unsigned char  v3_mail_data6;
	unsigned char  v3_mail_data7;
	unsigned char  v3_mail_data8;		/* 0xc8 */
	unsigned char  v3_mail_data9;
	unsigned char  v3_mail_data10;
	unsigned char  v3_mail_data11;
	unsigned char  v3_mail_data12;		/* 0xcc */
	unsigned char  v3_mail_data13;
	unsigned char  v3_mail_data14;
	unsigned char  v3_mail_data15;
	unsigned short v3_pci_mail_iewr;	/* 0xd0 */
	unsigned short v3_pci_mail_ierd;
	unsigned short v3_lb_mail_iewr;		/* 0xd4 */
	unsigned short v3_lb_mail_ierd;
	unsigned short v3_mail_wr_stat;		/* 0xd8 */
	unsigned short v3_mail_rd_stat;
	unsigned long  v3_qba_map;		/* 0xdc */
	unsigned long  v3_dma_delay:8;		/* 0xe0 */
	unsigned long  reserved8:24;
	unsigned long  reserved9[7];		/* 0xe4 */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned short v3_pci_device;		/* 0x00 */
	unsigned short v3_pci_vendor;
	unsigned short v3_pci_stat;		/* 0x04 */
	unsigned short v3_pci_cmd;
	unsigned long  v3_pci_cc_rev;		/* 0x08 */
	unsigned long  v3_pci_hdr_cfg;		/* 0x0c */
	unsigned long  v3_pci_io_base;		/* 0x10 */
	unsigned long  v3_pci_base0;		/* 0x14 */
	unsigned long  v3_pci_base1;		/* 0x18 */
	unsigned long  reserved1[4];		/* 0x1c */
	unsigned short v3_pci_sub_id;		/* 0x2c */
	unsigned short v3_pci_sub_vendor;
	unsigned long  v3_pci_rom;		/* 0x30 */
	unsigned long  reserved2[2];		/* 0x34 */
	unsigned long  v3_pci_bparam;		/* 0x3c */
	unsigned long  v3_pci_map0;		/* 0x40 */
	unsigned long  v3_pci_map1;		/* 0x44 */
	unsigned long  v3_pci_int_stat;		/* 0x48 */
	unsigned long  v3_pci_int_cfg;		/* 0x4c */
	unsigned long  reserved3;		/* 0x50 */
	unsigned long  v3_lb_base0;		/* 0x54 */
	unsigned long  v3_lb_base1;		/* 0x58 */
	unsigned short v3_lb_map0;		/* 0x5c */
	unsigned short reserved4;
	unsigned short v3_lb_map1;		/* 0x60 */
	unsigned short reserved5;
	unsigned short v3_lb_map2;		/* 0x64 */
	unsigned short v3_lb_base2;
	unsigned long  v3_lb_size;		/* 0x68 */
	unsigned short v3_lb_io_base;		/* 0x6c */
	unsigned short reserved6;
	unsigned short v3_fifo_priority;	/* 0x70 */
	unsigned short v3_fifo_cfg;
	unsigned char  v3_lb_imask;		/* 0x74 */
	unsigned char  v3_lb_istat;
	unsigned short v3_fifo_stat;
	unsigned short v3_lb_cfg;		/* 0x78 */
	unsigned short v3_system;
	unsigned short reserved7;		/* 0x7c */
	unsigned short v3_pci_cfg;
	unsigned long  v3_dma_pci_addr0;	/* 0x80 */
	unsigned long  v3_dma_local_addr0;	/* 0x84 */
	unsigned long  v3_dma_csr0:8;		/* 0x88 */
	unsigned long  v3_dma_length0:24;
	unsigned long  v3_dma_ctlb_adr0;	/* 0x8c */
	unsigned long  v3_dma_pci_addr1;	/* 0x90 */
	unsigned long  v3_dma_local_addr1;	/* 0x94 */
	unsigned long  v3_dma_csr1:8;		/* 0x98 */
	unsigned long  v3_dma_length1:24;
	unsigned long  v3_dma_ctlb_adr1;	/* 0x9c */
	unsigned long  v3_i20_mups[8];		/* 0xa0 */
	unsigned char  v3_mail_data3;		/* 0xc0 */
	unsigned char  v3_mail_data2;
	unsigned char  v3_mail_data1;
	unsigned char  v3_mail_data0;
	unsigned char  v3_mail_data7;		/* 0xc4 */
	unsigned char  v3_mail_data6;
	unsigned char  v3_mail_data5;
	unsigned char  v3_mail_data4;
	unsigned char  v3_mail_data11;		/* 0xc8 */
	unsigned char  v3_mail_data10;
	unsigned char  v3_mail_data9;
	unsigned char  v3_mail_data8;
	unsigned char  v3_mail_data15;		/* 0xcc */
	unsigned char  v3_mail_data14;
	unsigned char  v3_mail_data13;
	unsigned char  v3_mail_data12;
	unsigned short v3_pci_mail_ierd;	/* 0xd0 */
	unsigned short v3_pci_mail_iewr;
	unsigned short v3_lb_mail_ierd;		/* 0xd4 */
	unsigned short v3_lb_mail_iewr;
	unsigned short v3_mail_rd_stat;		/* 0xd8 */
	unsigned short v3_mail_wr_stat;
	unsigned long  v3_qba_map;		/* 0xdc */
	unsigned long  reserved8:24;		/* 0xe0 */
	unsigned long  v3_dma_delay:8;
	unsigned long  reserved9[7];		/* 0xe4 */
#endif
    }						/* 0x100 */
cma_mb_v360epc;

#endif

#endif	/* __ASSEMBLY__ */

#endif	/* _COGENT_MB_H */

/*
 * Copyright (C) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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
#include <config.h>
#include <version.h>
#include <asm/processor.h>
#include <asm/macro.h>

#include <asm/processor.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	wait_timer	WAIT_200US
	wait_timer	WAIT_200US

	/*------- LBSC -------*/
	write32 MMSELR_A,	MMSELR_D

	/*------- DBSC2 -------*/
	write32 DBSC2_DBCONF_A,	DBSC2_DBCONF_D
	write32 DBSC2_DBTR0_A,	DBSC2_DBTR0_D
	write32 DBSC2_DBTR1_A,	DBSC2_DBTR1_D
	write32 DBSC2_DBTR2_A,	DBSC2_DBTR2_D
	write32 DBSC2_DBFREQ_A,	DBSC2_DBFREQ_D1
	write32 DBSC2_DBFREQ_A,	DBSC2_DBFREQ_D2
	wait_timer	WAIT_200US

	write32 DBSC2_DBDICODTOCD_A,	DBSC2_DBDICODTOCD_D
	write32 DBSC2_DBCMDCNT_A,	DBSC2_DBCMDCNT_D_CKE_H
	wait_timer	WAIT_200US
	write32 DBSC2_DBCMDCNT_A,	DBSC2_DBCMDCNT_D_PALL
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_EMRS2
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_EMRS3
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_EMRS1_1
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_MRS_1
	write32 DBSC2_DBCMDCNT_A,	DBSC2_DBCMDCNT_D_PALL
	write32 DBSC2_DBCMDCNT_A,	DBSC2_DBCMDCNT_D_REF
	write32 DBSC2_DBCMDCNT_A,	DBSC2_DBCMDCNT_D_REF
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_MRS_2
	wait_timer	WAIT_200US

	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_EMRS1_2
	write32 DBSC2_DBMRCNT_A,	DBSC2_DBMRCNT_D_EMRS1_1

	write32 DBSC2_DBEN_A,		DBSC2_DBEN_D
	write32 DBSC2_DBRFCNT1_A,	DBSC2_DBRFCNT1_D
	write32 DBSC2_DBRFCNT2_A,	DBSC2_DBRFCNT2_D
	write32 DBSC2_DBRFCNT0_A,	DBSC2_DBRFCNT0_D
	wait_timer	WAIT_200US

	/*------- GPIO -------*/
	write16 PACR_A,	PXCR_D
	write16 PBCR_A,	PXCR_D
	write16 PCCR_A,	PXCR_D
	write16 PDCR_A,	PXCR_D
	write16 PECR_A,	PXCR_D
	write16 PFCR_A,	PXCR_D
	write16 PGCR_A,	PXCR_D
	write16 PHCR_A,	PHCR_D
	write16 PJCR_A,	PJCR_D
	write16 PKCR_A,	PKCR_D
	write16 PLCR_A,	PXCR_D
	write16 PMCR_A,	PMCR_D
	write16 PNCR_A,	PNCR_D
	write16 PPCR_A,	PXCR_D
	write16 PQCR_A,	PXCR_D
	write16 PRCR_A,	PXCR_D

	write8	PEPUPR_A,	PEPUPR_D
	write8	PHPUPR_A,	PHPUPR_D
	write8	PJPUPR_A,	PJPUPR_D
	write8	PKPUPR_A,	PKPUPR_D
	write8	PLPUPR_A,	PLPUPR_D
	write8	PMPUPR_A,	PMPUPR_D
	write8	PNPUPR_A,	PNPUPR_D
	write16	PPUPR1_A,	PPUPR1_D
	write16	PPUPR2_A,	PPUPR2_D
	write16	P1MSELR_A,	P1MSELR_D
	write16	P2MSELR_A,	P2MSELR_D

	/*------- LBSC -------*/
	write32	BCR_A,		BCR_D
	write32	CS0BCR_A,	CS0BCR_D
	write32	CS0WCR_A,	CS0WCR_D
	write32	CS1BCR_A,	CS1BCR_D
	write32	CS1WCR_A,	CS1WCR_D
	write32	CS4BCR_A,	CS4BCR_D
	write32	CS4WCR_A,	CS4WCR_D

	mov.l	PASCR_A, r0
	mov.l	@r0, r2
	mov.l	PASCR_32BIT_MODE, r1
	tst	r1, r2
	bt	lbsc_29bit

	write32	CS2BCR_A,	CS_USB_BCR_D
	write32	CS2WCR_A,	CS_USB_WCR_D
	write32	CS3BCR_A,	CS_SD_BCR_D
	write32	CS3WCR_A,	CS_SD_WCR_D
	write32	CS5BCR_A,	CS_I2C_BCR_D
	write32	CS5WCR_A,	CS_I2C_WCR_D
	write32	CS6BCR_A,	CS0BCR_D
	write32	CS6WCR_A,	CS0WCR_D
	bra	lbsc_end
	 nop

lbsc_29bit:
	write32	CS5BCR_A,	CS_USB_BCR_D
	write32	CS5WCR_A,	CS_USB_WCR_D
	write32	CS6BCR_A,	CS_SD_BCR_D
	write32	CS6WCR_A,	CS_SD_WCR_D

lbsc_end:
#if defined(CONFIG_SH_32BIT)
	/*------- set PMB -------*/
	write32	PASCR_A,	PASCR_29BIT_D
	write32	MMUCR_A,	MMUCR_D

	/*****************************************************************
	 * ent	virt		phys		v	sz	c	wt
	 * 0	0xa0000000	0x00000000	1	64M	0	0
	 * 1	0xa4000000	0x04000000	1	16M	0	0
	 * 2	0xa6000000	0x08000000	1	16M	0	0
	 * 9	0x88000000	0x48000000	1	128M	1	1
	 * 10	0x90000000	0x50000000	1	128M	1	1
	 * 11	0x98000000	0x58000000	1	128M	1	1
	 * 13	0xa8000000	0x48000000	1	128M	0	0
	 * 14	0xb0000000	0x50000000	1	128M	0	0
	 * 15	0xb8000000	0x58000000	1	128M	0	0
	 */
	write32	PMB_ADDR_FLASH_A,	PMB_ADDR_FLASH_D
	write32	PMB_DATA_FLASH_A,	PMB_DATA_FLASH_D
	write32	PMB_ADDR_CPLD_A,	PMB_ADDR_CPLD_D
	write32	PMB_DATA_CPLD_A,	PMB_DATA_CPLD_D
	write32	PMB_ADDR_USB_A,		PMB_ADDR_USB_D
	write32	PMB_DATA_USB_A,		PMB_DATA_USB_D
	write32	PMB_ADDR_DDR_C1_A,	PMB_ADDR_DDR_C1_D
	write32	PMB_DATA_DDR_C1_A,	PMB_DATA_DDR_C1_D
	write32	PMB_ADDR_DDR_C2_A,	PMB_ADDR_DDR_C2_D
	write32	PMB_DATA_DDR_C2_A,	PMB_DATA_DDR_C2_D
	write32	PMB_ADDR_DDR_C3_A,	PMB_ADDR_DDR_C3_D
	write32	PMB_DATA_DDR_C3_A,	PMB_DATA_DDR_C3_D
	write32	PMB_ADDR_DDR_N1_A,	PMB_ADDR_DDR_N1_D
	write32	PMB_DATA_DDR_N1_A,	PMB_DATA_DDR_N1_D
	write32	PMB_ADDR_DDR_N2_A,	PMB_ADDR_DDR_N2_D
	write32	PMB_DATA_DDR_N2_A,	PMB_DATA_DDR_N2_D
	write32	PMB_ADDR_DDR_N3_A,	PMB_ADDR_DDR_N3_D
	write32	PMB_DATA_DDR_N3_A,	PMB_DATA_DDR_N3_D

	write32	PASCR_A,	PASCR_INIT
	mov.l	DUMMY_ADDR, r0
	icbi	@r0
#endif

	write32	CCR_A,	CCR_D

	rts
	nop

	.align 4

/*------- GPIO -------*/
/* P{A,B C,D,E,F,G,L,P,Q,R}CR_D */
PXCR_D:		.word	0x0000

PHCR_D:		.word	0x00c0
PJCR_D:		.word	0xc3fc
PKCR_D:		.word	0x03ff
PMCR_D:		.word	0xffff
PNCR_D:		.word	0xf0c3

PEPUPR_D:	.long	0xff
PHPUPR_D:	.long	0x00
PJPUPR_D:	.long	0x00
PKPUPR_D:	.long	0x00
PLPUPR_D:	.long	0x00
PMPUPR_D:	.long	0xfc
PNPUPR_D:	.long	0x00
PPUPR1_D:	.word	0xffbf
PPUPR2_D:	.word	0xff00
P1MSELR_D:	.word	0x3780
P2MSELR_D:	.word	0x0000

#define GPIO_BASE	0xffe70000
PACR_A:		.long	GPIO_BASE + 0x00
PBCR_A:		.long	GPIO_BASE + 0x02
PCCR_A:		.long	GPIO_BASE + 0x04
PDCR_A:		.long	GPIO_BASE + 0x06
PECR_A:		.long	GPIO_BASE + 0x08
PFCR_A:		.long	GPIO_BASE + 0x0a
PGCR_A:		.long	GPIO_BASE + 0x0c
PHCR_A:		.long	GPIO_BASE + 0x0e
PJCR_A:		.long	GPIO_BASE + 0x10
PKCR_A:		.long	GPIO_BASE + 0x12
PLCR_A:		.long	GPIO_BASE + 0x14
PMCR_A:		.long	GPIO_BASE + 0x16
PNCR_A:		.long	GPIO_BASE + 0x18
PPCR_A:		.long	GPIO_BASE + 0x1a
PQCR_A:		.long	GPIO_BASE + 0x1c
PRCR_A:		.long	GPIO_BASE + 0x1e
PEPUPR_A:	.long	GPIO_BASE + 0x48
PHPUPR_A:	.long	GPIO_BASE + 0x4e
PJPUPR_A:	.long	GPIO_BASE + 0x50
PKPUPR_A:	.long	GPIO_BASE + 0x52
PLPUPR_A:	.long	GPIO_BASE + 0x54
PMPUPR_A:	.long	GPIO_BASE + 0x56
PNPUPR_A:	.long	GPIO_BASE + 0x58
PPUPR1_A:	.long	GPIO_BASE + 0x60
PPUPR2_A:	.long	GPIO_BASE + 0x62
P1MSELR_A:	.long	GPIO_BASE + 0x80
P2MSELR_A:	.long	GPIO_BASE + 0x82

MMSELR_A:      .long   0xfc400020
#if defined(CONFIG_SH_32BIT)
MMSELR_D:      .long   0xa5a50005
#else
MMSELR_D:      .long   0xa5a50002
#endif

/*------- DBSC2 -------*/
#define DBSC2_BASE	0xfe800000
DBSC2_DBSTATE_A:	.long	DBSC2_BASE + 0x0c
DBSC2_DBEN_A:		.long	DBSC2_BASE + 0x10
DBSC2_DBCMDCNT_A:	.long	DBSC2_BASE + 0x14
DBSC2_DBCONF_A:		.long	DBSC2_BASE + 0x20
DBSC2_DBTR0_A:		.long	DBSC2_BASE + 0x30
DBSC2_DBTR1_A:		.long	DBSC2_BASE + 0x34
DBSC2_DBTR2_A:		.long	DBSC2_BASE + 0x38
DBSC2_DBRFCNT0_A:	.long	DBSC2_BASE + 0x40
DBSC2_DBRFCNT1_A:	.long	DBSC2_BASE + 0x44
DBSC2_DBRFCNT2_A:	.long	DBSC2_BASE + 0x48
DBSC2_DBRFSTS_A:	.long	DBSC2_BASE + 0x4c
DBSC2_DBFREQ_A:		.long	DBSC2_BASE + 0x50
DBSC2_DBDICODTOCD_A:.long	DBSC2_BASE + 0x54
DBSC2_DBMRCNT_A:	.long	DBSC2_BASE + 0x60
DDR_DUMMY_ACCESS_A:	.long	0x40000000

DBSC2_DBCONF_D:		.long	0x00630002
DBSC2_DBTR0_D:		.long	0x050b1f04
DBSC2_DBTR1_D:		.long	0x00040204
DBSC2_DBTR2_D:		.long	0x02100308
DBSC2_DBFREQ_D1:	.long	0x00000000
DBSC2_DBFREQ_D2:	.long	0x00000100
DBSC2_DBDICODTOCD_D:.long	0x000f0907

DBSC2_DBCMDCNT_D_CKE_H:	.long	0x00000003
DBSC2_DBCMDCNT_D_PALL:	.long	0x00000002
DBSC2_DBCMDCNT_D_REF:	.long	0x00000004

DBSC2_DBMRCNT_D_EMRS2:	.long	0x00020000
DBSC2_DBMRCNT_D_EMRS3:	.long	0x00030000
DBSC2_DBMRCNT_D_EMRS1_1:	.long	0x00010006
DBSC2_DBMRCNT_D_EMRS1_2:	.long	0x00010386
DBSC2_DBMRCNT_D_MRS_1:	.long	0x00000952
DBSC2_DBMRCNT_D_MRS_2:	.long	0x00000852

DBSC2_DBEN_D:		.long	0x00000001

DBSC2_DBPDCNT0_D3:	.long	0x00000080
DBSC2_DBRFCNT1_D:	.long	0x00000926
DBSC2_DBRFCNT2_D:	.long	0x00fe00fe
DBSC2_DBRFCNT0_D:	.long	0x00010000

WAIT_200US:    .long   33333

/*------- LBSC -------*/
PASCR_A:		.long	0xff000070
PASCR_32BIT_MODE:	.long	0x80000000	/* check booting mode */

BCR_A:		.long	BCR
CS0BCR_A:	.long	CS0BCR
CS0WCR_A:	.long	CS0WCR
CS1BCR_A:	.long	CS1BCR
CS1WCR_A:	.long	CS1WCR
CS2BCR_A:	.long	CS2BCR
CS2WCR_A:	.long	CS2WCR
CS3BCR_A:	.long	CS3BCR
CS3WCR_A:	.long	CS3WCR
CS4BCR_A:	.long	CS4BCR
CS4WCR_A:	.long	CS4WCR
CS5BCR_A:	.long	CS5BCR
CS5WCR_A:	.long	CS5WCR
CS6BCR_A:	.long	CS6BCR
CS6WCR_A:	.long	CS6WCR

BCR_D:		.long	0x80000003
CS0BCR_D:	.long	0x22222340
CS0WCR_D:	.long	0x00111118
CS1BCR_D:	.long	0x11111100
CS1WCR_D:	.long	0x33333303
CS4BCR_D:	.long	0x11111300
CS4WCR_D:	.long	0x00101012

/* USB setting : 32bit mode = CS2, 29bit mode = CS5 */
CS_USB_BCR_D:	.long	0x11111200
CS_USB_WCR_D:	.long	0x00020004

/* SD setting : 32bit mode = CS3, 29bit mode = CS6 */
CS_SD_BCR_D:	.long	0x00000300
CS_SD_WCR_D:	.long	0x00030108

/* I2C setting : 32bit mode = CS5, 29bit mode = CS1(already setting) */
CS_I2C_BCR_D:	.long	0x11111100
CS_I2C_WCR_D:	.long	0x00000003

#if defined(CONFIG_SH_32BIT)
/*------- set PMB -------*/
PMB_ADDR_FLASH_A:	.long	PMB_ADDR_BASE(0)
PMB_ADDR_CPLD_A:	.long	PMB_ADDR_BASE(1)
PMB_ADDR_USB_A:		.long	PMB_ADDR_BASE(2)
PMB_ADDR_DDR_C1_A:	.long	PMB_ADDR_BASE(9)
PMB_ADDR_DDR_C2_A:	.long	PMB_ADDR_BASE(10)
PMB_ADDR_DDR_C3_A:	.long	PMB_ADDR_BASE(11)
PMB_ADDR_DDR_N1_A:	.long	PMB_ADDR_BASE(13)
PMB_ADDR_DDR_N2_A:	.long	PMB_ADDR_BASE(14)
PMB_ADDR_DDR_N3_A:	.long	PMB_ADDR_BASE(15)

PMB_ADDR_FLASH_D:	.long	mk_pmb_addr_val(0xa0)
PMB_ADDR_CPLD_D:	.long	mk_pmb_addr_val(0xa4)
PMB_ADDR_USB_D:		.long	mk_pmb_addr_val(0xa6)
PMB_ADDR_DDR_C1_D:	.long	mk_pmb_addr_val(0x88)
PMB_ADDR_DDR_C2_D:	.long	mk_pmb_addr_val(0x90)
PMB_ADDR_DDR_C3_D:	.long	mk_pmb_addr_val(0x98)
PMB_ADDR_DDR_N1_D:	.long	mk_pmb_addr_val(0xa8)
PMB_ADDR_DDR_N2_D:	.long	mk_pmb_addr_val(0xb0)
PMB_ADDR_DDR_N3_D:	.long	mk_pmb_addr_val(0xb8)

PMB_DATA_FLASH_A:	.long	PMB_DATA_BASE(0)
PMB_DATA_CPLD_A:	.long	PMB_DATA_BASE(1)
PMB_DATA_USB_A:		.long	PMB_DATA_BASE(2)
PMB_DATA_DDR_C1_A:	.long	PMB_DATA_BASE(9)
PMB_DATA_DDR_C2_A:	.long	PMB_DATA_BASE(10)
PMB_DATA_DDR_C3_A:	.long	PMB_DATA_BASE(11)
PMB_DATA_DDR_N1_A:	.long	PMB_DATA_BASE(13)
PMB_DATA_DDR_N2_A:	.long	PMB_DATA_BASE(14)
PMB_DATA_DDR_N3_A:	.long	PMB_DATA_BASE(15)

/*						ppn   ub v s1 s0  c  wt */
PMB_DATA_FLASH_D:	.long	mk_pmb_data_val(0x00, 1, 1, 0, 1, 0, 1)
PMB_DATA_CPLD_D:	.long	mk_pmb_data_val(0x04, 1, 1, 0, 0, 0, 1)
PMB_DATA_USB_D:		.long	mk_pmb_data_val(0x08, 1, 1, 0, 0, 0, 1)
PMB_DATA_DDR_C1_D:	.long	mk_pmb_data_val(0x48, 0, 1, 1, 0, 1, 1)
PMB_DATA_DDR_C2_D:	.long	mk_pmb_data_val(0x50, 0, 1, 1, 0, 1, 1)
PMB_DATA_DDR_C3_D:	.long	mk_pmb_data_val(0x58, 0, 1, 1, 0, 1, 1)
PMB_DATA_DDR_N1_D:	.long	mk_pmb_data_val(0x48, 1, 1, 1, 0, 0, 1)
PMB_DATA_DDR_N2_D:	.long	mk_pmb_data_val(0x50, 1, 1, 1, 0, 0, 1)
PMB_DATA_DDR_N3_D:	.long	mk_pmb_data_val(0x58, 1, 1, 1, 0, 0, 1)

DUMMY_ADDR:	.long	0xa0000000
PASCR_29BIT_D:	.long	0x00000000
PASCR_INIT:	.long	0x80000080	/* check booting mode */
MMUCR_A:	.long	0xff000010
MMUCR_D:	.long	0x00000004	/* clear ITLB */
#endif	/* CONFIG_SH_32BIT */

CCR_A:		.long	0xff00001c
CCR_D:		.long	0x0000090b

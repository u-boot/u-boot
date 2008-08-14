/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
 * (C) Copyright 2007 DENX Software Engineering
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Derived from the MPC83xx header.
 */

#ifndef __MPC512X_H__
#define __MPC512X_H__

#include <config.h>
#if defined(CONFIG_E300)
#include <asm/e300.h>
#endif

/* System reset offset (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET		0x0100
#define	_START_OFFSET			EXC_OFF_SYS_RESET


/* IMMRBAR - Internal Memory Register Base Address
 */
#define CONFIG_DEFAULT_IMMR		0xFF400000	/* Default IMMR base address */
#define IMMRBAR				0x0000		/* Register offset to immr */
#define IMMRBAR_BASE_ADDR		0xFFF00000	/* Base address mask */
#define IMMRBAR_RES			~(IMMRBAR_BASE_ADDR)

/* LAWBAR - Local Access Window Base Address Register
 */
#define LPBAW			0x0020		/* Register offset to immr */
#define LPCS0AW			0x0024
#define LPCS1AW			0x0028
#define LPCS2AW			0x002C
#define LPCS3AW			0x0030
#define LPCS4AW			0x0034
#define LPCS5AW			0x0038
#define LPCS6AW			0x003C
#define LPCA7AW			0x0040
#define SRAMBAR			0x00C4
#define LAWBAR_BAR		0xFFFFF000	/* Base address mask */

#define LPC_OFFSET		0x10000

#define CS0_CONFIG		0x00000
#define CS1_CONFIG		0x00004
#define CS2_CONFIG		0x00008
#define CS3_CONFIG		0x0000C
#define CS4_CONFIG		0x00010
#define CS5_CONFIG		0x00014
#define CS6_CONFIG		0x00018
#define CS7_CONFIG		0x0001C

#define CS_CTRL			0x00020
#define CS_CTRL_ME		0x01000000	/* CS Master Enable bit */
#define CS_CTRL_IE		0x08000000	/* CS Interrupt Enable bit */

/* SPRIDR - System Part and Revision ID Register
 */
#define SPRIDR_PARTID		0xFFFF0000	/* Part Identification */
#define SPRIDR_REVID		0x0000FFFF	/* Revision Identification */

#define SPR_5121E		0x80180000

/* SPCR - System Priority Configuration Register
 */
#define SPCR_PCIHPE			0x10000000	/* PCI Highest Priority Enable */
#define SPCR_PCIHPE_SHIFT		(31-3)
#define SPCR_PCIPR			0x03000000	/* PCI bridge system bus request priority */
#define SPCR_PCIPR_SHIFT		(31-7)
#define SPCR_TBEN			0x00400000	/* E300 PowerPC core time base unit enable */
#define SPCR_TBEN_SHIFT			(31-9)
#define SPCR_COREPR			0x00300000	/* E300 PowerPC Core system bus request priority */
#define SPCR_COREPR_SHIFT		(31-11)

/* SWCRR - System Watchdog Control Register
 */
#define SWCRR				0x0904		/* Register offset to immr */
#define SWCRR_SWTC			0xFFFF0000	/* Software Watchdog Time Count */
#define SWCRR_SWEN			0x00000004	/* Watchdog Enable bit */
#define SWCRR_SWRI			0x00000002	/* Software Watchdog Reset/Interrupt Select bit */
#define SWCRR_SWPR			0x00000001	/* Software Watchdog Counter Prescale bit */
#define SWCRR_RES			~(SWCRR_SWTC | SWCRR_SWEN | SWCRR_SWRI | SWCRR_SWPR)

/* SWCNR - System Watchdog Counter Register
 */
#define SWCNR				0x0908		/* Register offset to immr */
#define SWCNR_SWCN			0x0000FFFF	/* Software Watchdog Count mask */
#define SWCNR_RES			~(SWCNR_SWCN)

/* SWSRR - System Watchdog Service Register
 */
#define SWSRR				0x090E		/* Register offset to immr */

/* ACR - Arbiter Configuration Register
 */
#define ACR_COREDIS			0x10000000	/* Core disable */
#define ACR_COREDIS_SHIFT		(31-7)
#define ACR_PIPE_DEP			0x00070000	/* Pipeline depth */
#define ACR_PIPE_DEP_SHIFT		(31-15)
#define ACR_PCI_RPTCNT			0x00007000	/* PCI repeat count */
#define ACR_PCI_RPTCNT_SHIFT		(31-19)
#define ACR_RPTCNT			0x00000700	/* Repeat count */
#define ACR_RPTCNT_SHIFT		(31-23)
#define ACR_APARK			0x00000030	/* Address parking */
#define ACR_APARK_SHIFT			(31-27)
#define ACR_PARKM			0x0000000F	/* Parking master */
#define ACR_PARKM_SHIFT			(31-31)

/* ATR - Arbiter Timers Register
 */
#define ATR_DTO				0x00FF0000	/* Data time out */
#define ATR_ATO				0x000000FF	/* Address time out */

/* AER - Arbiter Event Register
 */
#define AER_ETEA			0x00000020	/* Transfer error */
#define AER_RES				0x00000010	/* Reserved transfer type */
#define AER_ECW				0x00000008	/* External control word transfer type */
#define AER_AO				0x00000004	/* Address Only transfer type */
#define AER_DTO				0x00000002	/* Data time out */
#define AER_ATO				0x00000001	/* Address time out */

/* AEATR - Arbiter Event Address Register
 */
#define AEATR_EVENT			0x07000000	/* Event type */
#define AEATR_MSTR_ID			0x001F0000	/* Master Id */
#define AEATR_TBST			0x00000800	/* Transfer burst */
#define AEATR_TSIZE			0x00000700	/* Transfer Size */
#define AEATR_TTYPE			0x0000001F	/* Transfer Type */

/* RSR - Reset Status Register
 */
#define RSR_SWSR			0x00002000	/* software soft reset */
#define RSR_SWSR_SHIFT			13
#define RSR_SWHR			0x00001000	/* software hard reset */
#define RSR_SWHR_SHIFT			12
#define RSR_JHRS			0x00000200	/* jtag hreset */
#define RSR_JHRS_SHIFT			9
#define RSR_JSRS			0x00000100	/* jtag sreset status */
#define RSR_JSRS_SHIFT			8
#define RSR_CSHR			0x00000010	/* checkstop reset status */
#define RSR_CSHR_SHIFT			4
#define RSR_SWRS			0x00000008	/* software watchdog reset status */
#define RSR_SWRS_SHIFT			3
#define RSR_BMRS			0x00000004	/* bus monitop reset status */
#define RSR_BMRS_SHIFT			2
#define RSR_SRS				0x00000002	/* soft reset status */
#define RSR_SRS_SHIFT			1
#define RSR_HRS				0x00000001	/* hard reset status */
#define RSR_HRS_SHIFT			0
#define RSR_RES				~(RSR_SWSR | RSR_SWHR |\
					 RSR_JHRS | RSR_JSRS | RSR_CSHR | RSR_SWRS |\
					 RSR_BMRS | RSR_SRS | RSR_HRS)
/* RMR - Reset Mode Register
 */
#define RMR_CSRE			0x00000001	/* checkstop reset enable */
#define RMR_CSRE_SHIFT			0
#define RMR_RES				~(RMR_CSRE)

/* RCR - Reset Control Register
 */
#define RCR_SWHR			0x00000002	/* software hard reset */
#define RCR_SWSR			0x00000001	/* software soft reset */
#define RCR_RES				~(RCR_SWHR | RCR_SWSR)

/* RCER - Reset Control Enable Register
 */
#define RCER_CRE			0x00000001	/* software hard reset */
#define RCER_RES			~(RCER_CRE)

/* SPMR - System PLL Mode Register
 */
#define SPMR_SPMF			0x0F000000
#define SPMR_SPMF_SHIFT			24
#define SPMR_CPMF			0x000F0000
#define SPMR_CPMF_SHIFT			16

/* SCFR1 System Clock Frequency Register 1
 */
#define SCFR1_IPS_DIV			0x3
#define SCFR1_IPS_DIV_MASK		0x03800000
#define SCFR1_IPS_DIV_SHIFT		23

#define SCFR1_PCI_DIV			0x6
#define SCFR1_PCI_DIV_MASK		0x00700000
#define SCFR1_PCI_DIV_SHIFT		20

/* SCFR2 System Clock Frequency Register 2
 */
#define SCFR2_SYS_DIV			0xFC000000
#define SCFR2_SYS_DIV_SHIFT		26

/* SCCR - System Clock Control Registers
 */

/* System Clock Control Register 1 commands */
#define CLOCK_SCCR1_CFG_EN		0x80000000
#define CLOCK_SCCR1_LPC_EN		0x40000000
#define CLOCK_SCCR1_NFC_EN		0x20000000
#define CLOCK_SCCR1_PATA_EN		0x10000000
#define CLOCK_SCCR1_PSC_EN(cn)		(0x08000000 >> (cn))
#define CLOCK_SCCR1_PSCFIFO_EN		0x00008000
#define CLOCK_SCCR1_SATA_EN		0x00004000
#define CLOCK_SCCR1_FEC_EN		0x00002000
#define CLOCK_SCCR1_TPR_EN		0x00001000
#define CLOCK_SCCR1_PCI_EN		0x00000800
#define CLOCK_SCCR1_DDR_EN		0x00000400

/* System Clock Control Register 2 commands */
#define CLOCK_SCCR2_DIU_EN		0x80000000
#define CLOCK_SCCR2_AXE_EN		0x40000000
#define CLOCK_SCCR2_MEM_EN		0x20000000
#define CLOCK_SCCR2_USB2_EN		0x10000000
#define CLOCK_SCCR2_USB1_EN		0x08000000
#define CLOCK_SCCR2_I2C_EN		0x04000000
#define CLOCK_SCCR2_BDLC_EN		0x02000000
#define CLOCK_SCCR2_SDHC_EN		0x01000000
#define CLOCK_SCCR2_SPDIF_EN		0x00800000
#define CLOCK_SCCR2_MBX_BUS_EN		0x00400000
#define CLOCK_SCCR2_MBX_EN		0x00200000
#define CLOCK_SCCR2_MBX_3D_EN		0x00100000
#define CLOCK_SCCR2_IIM_EN		0x00080000

/* PSC FIFO Command values */
#define PSC_FIFO_RESET_SLICE		0x80
#define PSC_FIFO_ENABLE_SLICE		0x01

/* PSC FIFO Controller Command values */
#define FIFOC_ENABLE_CLOCK_GATE		0x01
#define FIFOC_DISABLE_CLOCK_GATE	0x00

/* PSC FIFO status */
#define PSC_FIFO_EMPTY			0x01

/* PSC Command values */
#define PSC_RX_ENABLE		0x01
#define PSC_RX_DISABLE		0x02
#define PSC_TX_ENABLE		0x04
#define PSC_TX_DISABLE		0x08
#define PSC_SEL_MODE_REG_1	0x10
#define PSC_RST_RX		0x20
#define PSC_RST_TX		0x30
#define PSC_RST_ERR_STAT	0x40
#define PSC_RST_BRK_CHG_INT	0x50
#define PSC_START_BRK		0x60
#define PSC_STOP_BRK		0x70

/* PSC status register bits */
#define PSC_SR_CDE		0x0080
#define PSC_SR_TXEMP		0x0800
#define PSC_SR_OE		0x1000
#define PSC_SR_PE		0x2000
#define PSC_SR_FE		0x4000
#define PSC_SR_RB		0x8000

/* PSC mode fields */
#define PSC_MODE_5_BITS		0x00
#define PSC_MODE_6_BITS		0x01
#define PSC_MODE_7_BITS		0x02
#define PSC_MODE_8_BITS		0x03
#define PSC_MODE_PAREVEN	0x00
#define PSC_MODE_PARODD		0x04
#define PSC_MODE_PARFORCE	0x08
#define PSC_MODE_PARNONE	0x10
#define PSC_MODE_ENTIMEOUT	0x20
#define PSC_MODE_RXRTS		0x80
#define PSC_MODE_1_STOPBIT	0x07

/*
 * Centralized FIFO Controller has internal memory for all 12 PSCs FIFOs
 *
 * NOTE: individual PSC units are free to use whatever area (and size) of the
 * FIFOC internal memory, so make sure memory areas for FIFO slices used by
 * different PSCs do not overlap!
 *
 * Overall size of FIFOC memory is not documented in the MPC5121e RM, but
 * tests indicate that it is 1024 words total.
 */
#define FIFOC_PSC0_TX_SIZE	0x0	/* number of 4-byte words for FIFO slice */
#define FIFOC_PSC0_TX_ADDR	0x0
#define FIFOC_PSC0_RX_SIZE	0x0
#define FIFOC_PSC0_RX_ADDR	0x0

#define FIFOC_PSC1_TX_SIZE	0x0
#define FIFOC_PSC1_TX_ADDR	0x0
#define FIFOC_PSC1_RX_SIZE	0x0
#define FIFOC_PSC1_RX_ADDR	0x0

#define FIFOC_PSC2_TX_SIZE	0x0
#define FIFOC_PSC2_TX_ADDR	0x0
#define FIFOC_PSC2_RX_SIZE	0x0
#define FIFOC_PSC2_RX_ADDR	0x0

#define FIFOC_PSC3_TX_SIZE	0x04
#define FIFOC_PSC3_TX_ADDR	0x0
#define FIFOC_PSC3_RX_SIZE	0x04
#define FIFOC_PSC3_RX_ADDR	0x10

#define FIFOC_PSC4_TX_SIZE	0x0
#define FIFOC_PSC4_TX_ADDR	0x0
#define FIFOC_PSC4_RX_SIZE	0x0
#define FIFOC_PSC4_RX_ADDR	0x0

#define FIFOC_PSC5_TX_SIZE	0x0
#define FIFOC_PSC5_TX_ADDR	0x0
#define FIFOC_PSC5_RX_SIZE	0x0
#define FIFOC_PSC5_RX_ADDR	0x0

#define FIFOC_PSC6_TX_SIZE	0x0
#define FIFOC_PSC6_TX_ADDR	0x0
#define FIFOC_PSC6_RX_SIZE	0x0
#define FIFOC_PSC6_RX_ADDR	0x0

#define FIFOC_PSC7_TX_SIZE	0x0
#define FIFOC_PSC7_TX_ADDR	0x0
#define FIFOC_PSC7_RX_SIZE	0x0
#define FIFOC_PSC7_RX_ADDR	0x0

#define FIFOC_PSC8_TX_SIZE	0x0
#define FIFOC_PSC8_TX_ADDR	0x0
#define FIFOC_PSC8_RX_SIZE	0x0
#define FIFOC_PSC8_RX_ADDR	0x0

#define FIFOC_PSC9_TX_SIZE	0x0
#define FIFOC_PSC9_TX_ADDR	0x0
#define FIFOC_PSC9_RX_SIZE	0x0
#define FIFOC_PSC9_RX_ADDR	0x0

#define FIFOC_PSC10_TX_SIZE	0x0
#define FIFOC_PSC10_TX_ADDR	0x0
#define FIFOC_PSC10_RX_SIZE	0x0
#define FIFOC_PSC10_RX_ADDR	0x0

#define FIFOC_PSC11_TX_SIZE	0x0
#define FIFOC_PSC11_TX_ADDR	0x0
#define FIFOC_PSC11_RX_SIZE	0x0
#define FIFOC_PSC11_RX_ADDR	0x0

/* IO Control Register
 */
#define IOCTL_MEM		0x000
#define IOCTL_GP		0x004
#define IOCTL_LPC_CLK		0x008
#define IOCTL_LPC_OE		0x00C
#define IOCTL_LPC_RWB		0x010
#define IOCTL_LPC_ACK		0x014
#define IOCTL_LPC_CS0		0x018
#define IOCTL_NFC_CE0		0x01C
#define IOCTL_LPC_CS1		0x020
#define IOCTL_LPC_CS2		0x024
#define IOCTL_LPC_AX03		0x028
#define IOCTL_EMB_AX02		0x02C
#define IOCTL_EMB_AX01		0x030
#define IOCTL_EMB_AX00		0x034
#define IOCTL_EMB_AD31		0x038
#define IOCTL_EMB_AD30		0x03C
#define IOCTL_EMB_AD29		0x040
#define IOCTL_EMB_AD28		0x044
#define IOCTL_EMB_AD27		0x048
#define IOCTL_EMB_AD26		0x04C
#define IOCTL_EMB_AD25		0x050
#define IOCTL_EMB_AD24		0x054
#define IOCTL_EMB_AD23		0x058
#define IOCTL_EMB_AD22		0x05C
#define IOCTL_EMB_AD21		0x060
#define IOCTL_EMB_AD20		0x064
#define IOCTL_EMB_AD19		0x068
#define IOCTL_EMB_AD18		0x06C
#define IOCTL_EMB_AD17		0x070
#define IOCTL_EMB_AD16		0x074
#define IOCTL_EMB_AD15		0x078
#define IOCTL_EMB_AD14		0x07C
#define IOCTL_EMB_AD13		0x080
#define IOCTL_EMB_AD12		0x084
#define IOCTL_EMB_AD11		0x088
#define IOCTL_EMB_AD10		0x08C
#define IOCTL_EMB_AD09		0x090
#define IOCTL_EMB_AD08		0x094
#define IOCTL_EMB_AD07		0x098
#define IOCTL_EMB_AD06		0x09C
#define IOCTL_EMB_AD05		0x0A0
#define IOCTL_EMB_AD04		0x0A4
#define IOCTL_EMB_AD03		0x0A8
#define IOCTL_EMB_AD02		0x0AC
#define IOCTL_EMB_AD01		0x0B0
#define IOCTL_EMB_AD00		0x0B4
#define IOCTL_PATA_CE1		0x0B8
#define IOCTL_PATA_CE2		0x0BC
#define IOCTL_PATA_ISOLATE	0x0C0
#define IOCTL_PATA_IOR		0x0C4
#define IOCTL_PATA_IOW		0x0C8
#define IOCTL_PATA_IOCHRDY	0x0CC
#define IOCTL_PATA_INTRQ	0x0D0
#define IOCTL_PATA_DRQ		0x0D4
#define IOCTL_PATA_DACK		0x0D8
#define IOCTL_NFC_WP		0x0DC
#define IOCTL_NFC_RB		0x0E0
#define IOCTL_NFC_ALE		0x0E4
#define IOCTL_NFC_CLE		0x0E8
#define IOCTL_NFC_WE		0x0EC
#define IOCTL_NFC_RE		0x0F0
#define IOCTL_PCI_AD31		0x0F4
#define IOCTL_PCI_AD30		0x0F8
#define IOCTL_PCI_AD29		0x0FC
#define IOCTL_PCI_AD28		0x100
#define IOCTL_PCI_AD27		0x104
#define IOCTL_PCI_AD26		0x108
#define IOCTL_PCI_AD25		0x10C
#define IOCTL_PCI_AD24		0x110
#define IOCTL_PCI_AD23		0x114
#define IOCTL_PCI_AD22		0x118
#define IOCTL_PCI_AD21		0x11C
#define IOCTL_PCI_AD20		0x120
#define IOCTL_PCI_AD19		0x124
#define IOCTL_PCI_AD18		0x128
#define IOCTL_PCI_AD17		0x12C
#define IOCTL_PCI_AD16		0x130
#define IOCTL_PCI_AD15		0x134
#define IOCTL_PCI_AD14		0x138
#define IOCTL_PCI_AD13		0x13C
#define IOCTL_PCI_AD12		0x140
#define IOCTL_PCI_AD11		0x144
#define IOCTL_PCI_AD10		0x148
#define IOCTL_PCI_AD09		0x14C
#define IOCTL_PCI_AD08		0x150
#define IOCTL_PCI_AD07		0x154
#define IOCTL_PCI_AD06		0x158
#define IOCTL_PCI_AD05		0x15C
#define IOCTL_PCI_AD04		0x160
#define IOCTL_PCI_AD03		0x164
#define IOCTL_PCI_AD02		0x168
#define IOCTL_PCI_AD01		0x16C
#define IOCTL_PCI_AD00		0x170
#define IOCTL_PCI_CBE0		0x174
#define IOCTL_PCI_CBE1		0x178
#define IOCTL_PCI_CBE2		0x17C
#define IOCTL_PCI_CBE3		0x180
#define IOCTL_PCI_GNT2		0x184
#define IOCTL_PCI_REQ2		0x188
#define IOCTL_PCI_GNT1		0x18C
#define IOCTL_PCI_REQ1		0x190
#define IOCTL_PCI_GNT0		0x194
#define IOCTL_PCI_REQ0		0x198
#define IOCTL_PCI_INTA		0x19C
#define IOCTL_PCI_CLK		0x1A0
#define IOCTL_PCI_RST_OUT	0x1A4
#define IOCTL_PCI_FRAME		0x1A8
#define IOCTL_PCI_IDSEL		0x1AC
#define IOCTL_PCI_DEVSEL	0x1B0
#define IOCTL_PCI_IRDY		0x1B4
#define IOCTL_PCI_TRDY		0x1B8
#define IOCTL_PCI_STOP		0x1BC
#define IOCTL_PCI_PAR		0x1C0
#define IOCTL_PCI_PERR		0x1C4
#define IOCTL_PCI_SERR		0x1C8
#define IOCTL_SPDIF_TXCLK	0x1CC
#define IOCTL_SPDIF_TX		0x1D0
#define IOCTL_SPDIF_RX		0x1D4
#define IOCTL_I2C0_SCL		0x1D8
#define IOCTL_I2C0_SDA		0x1DC
#define IOCTL_I2C1_SCL		0x1E0
#define IOCTL_I2C1_SDA		0x1E4
#define IOCTL_I2C2_SCL		0x1E8
#define IOCTL_I2C2_SDA		0x1EC
#define IOCTL_IRQ0		0x1F0
#define IOCTL_IRQ1		0x1F4
#define IOCTL_CAN1_TX		0x1F8
#define IOCTL_CAN2_TX		0x1FC
#define IOCTL_J1850_TX		0x200
#define IOCTL_J1850_RX		0x204
#define IOCTL_PSC_MCLK_IN	0x208
#define IOCTL_PSC0_0		0x20C
#define IOCTL_PSC0_1		0x210
#define IOCTL_PSC0_2		0x214
#define IOCTL_PSC0_3		0x218
#define IOCTL_PSC0_4		0x21C
#define IOCTL_PSC1_0		0x220
#define IOCTL_PSC1_1		0x224
#define IOCTL_PSC1_2		0x228
#define IOCTL_PSC1_3		0x22C
#define IOCTL_PSC1_4		0x230
#define IOCTL_PSC2_0		0x234
#define IOCTL_PSC2_1		0x238
#define IOCTL_PSC2_2		0x23C
#define IOCTL_PSC2_3		0x240
#define IOCTL_PSC2_4		0x244
#define IOCTL_PSC3_0		0x248
#define IOCTL_PSC3_1		0x24C
#define IOCTL_PSC3_2		0x250
#define IOCTL_PSC3_3		0x254
#define IOCTL_PSC3_4		0x258
#define IOCTL_PSC4_0		0x25C
#define IOCTL_PSC4_1		0x260
#define IOCTL_PSC4_2		0x264
#define IOCTL_PSC4_3		0x268
#define IOCTL_PSC4_4		0x26C
#define IOCTL_PSC5_0		0x270
#define IOCTL_PSC5_1		0x274
#define IOCTL_PSC5_2		0x278
#define IOCTL_PSC5_3		0x27C
#define IOCTL_PSC5_4		0x280
#define IOCTL_PSC6_0		0x284
#define IOCTL_PSC6_1		0x288
#define IOCTL_PSC6_2		0x28C
#define IOCTL_PSC6_3		0x290
#define IOCTL_PSC6_4		0x294
#define IOCTL_PSC7_0		0x298
#define IOCTL_PSC7_1		0x29C
#define IOCTL_PSC7_2		0x2A0
#define IOCTL_PSC7_3		0x2A4
#define IOCTL_PSC7_4		0x2A8
#define IOCTL_PSC8_0		0x2AC
#define IOCTL_PSC8_1		0x2B0
#define IOCTL_PSC8_2		0x2B4
#define IOCTL_PSC8_3		0x2B8
#define IOCTL_PSC8_4		0x2BC
#define IOCTL_PSC9_0		0x2C0
#define IOCTL_PSC9_1		0x2C4
#define IOCTL_PSC9_2		0x2C8
#define IOCTL_PSC9_3		0x2CC
#define IOCTL_PSC9_4		0x2D0
#define IOCTL_PSC10_0		0x2D4
#define IOCTL_PSC10_1		0x2D8
#define IOCTL_PSC10_2		0x2DC
#define IOCTL_PSC10_3		0x2E0
#define IOCTL_PSC10_4		0x2E4
#define IOCTL_PSC11_0		0x2E8
#define IOCTL_PSC11_1		0x2EC
#define IOCTL_PSC11_2		0x2F0
#define IOCTL_PSC11_3		0x2F4
#define IOCTL_PSC11_4		0x2F8
#define IOCTL_HRESET		0x2FC
#define IOCTL_SRESET		0x300
#define IOCTL_CKSTP_OUT		0x304
#define IOCTL_USB2_VBUS_PWR_FAULT	0x308
#define IOCTL_USB2_VBUS_PWR_SELECT	0x30C
#define IOCTL_USB2_PHY_DRVV_BUS		0x310

#ifndef __ASSEMBLY__


/* IO pin fields */
#define IO_PIN_FMUX(v)	((v) << 7)	/* pin function */
#define IO_PIN_HOLD(v)	((v) << 5)	/* hold time, pci only */
#define IO_PIN_PUD(v)	((v) << 4)	/* if PUE, 0=pull-down, 1=pull-up */
#define IO_PIN_PUE(v)	((v) << 3)	/* pull up/down enable */
#define IO_PIN_ST(v)	((v) << 2)	/* schmitt trigger */
#define IO_PIN_DS(v)	((v))		/* slew rate */

typedef struct iopin_t {
	int p_offset;		/* offset from IOCTL_MEM_OFFSET */
	int nr_pins;		/* number of pins to set this way */
	int bit_or;		/* or in the value instead of overwrite */
	u_long val;		/* value to write or or */
}iopin_t;

void iopin_initialize(iopin_t *,int);
#endif

/* Indexes in regs array */
/* Set for DDR */
#define IOCTRL_MUX_DDR		0x00000036

 /* Register Offset Base */
#define MPC512X_FEC		(CFG_IMMR + 0x02800)

/* Number of I2C buses */
#define I2C_BUS_CNT	3

/* I2Cn control register bits */
#define I2C_EN		0x80
#define I2C_IEN		0x40
#define I2C_STA		0x20
#define I2C_TX		0x10
#define I2C_TXAK	0x08
#define I2C_RSTA	0x04
#define I2C_INIT_MASK	(I2C_EN | I2C_STA | I2C_TX | I2C_RSTA)

/* I2Cn status register bits */
#define I2C_CF		0x80
#define I2C_AAS		0x40
#define I2C_BB		0x20
#define I2C_AL		0x10
#define I2C_SRW		0x04
#define I2C_IF		0x02
#define I2C_RXAK	0x01

/* POTAR - PCI Outbound Translation Address Register
 */
#define POTAR_TA_MASK			0x000fffff

/* POBAR - PCI Outbound Base Address Register
 */
#define POBAR_BA_MASK			0x000fffff

/* POCMR - PCI Outbound Comparision Mask Register
 */
#define POCMR_EN	0x80000000
#define POCMR_IO	0x40000000	/* 0-memory space 1-I/O space */
#define POCMR_PRE	0x20000000	/* prefetch enable */
#define POCMR_SBS	0x00100000	/* special byte swap enable */
#define POCMR_CM_MASK	0x000fffff
#define POCMR_CM_4G	0x00000000
#define POCMR_CM_2G	0x00080000
#define POCMR_CM_1G	0x000C0000
#define POCMR_CM_512M	0x000E0000
#define POCMR_CM_256M	0x000F0000
#define POCMR_CM_128M	0x000F8000
#define POCMR_CM_64M	0x000FC000
#define POCMR_CM_32M	0x000FE000
#define POCMR_CM_16M	0x000FF000
#define POCMR_CM_8M	0x000FF800
#define POCMR_CM_4M	0x000FFC00
#define POCMR_CM_2M	0x000FFE00
#define POCMR_CM_1M	0x000FFF00
#define POCMR_CM_512K	0x000FFF80
#define POCMR_CM_256K	0x000FFFC0
#define POCMR_CM_128K	0x000FFFE0
#define POCMR_CM_64K	0x000FFFF0
#define POCMR_CM_32K	0x000FFFF8
#define POCMR_CM_16K	0x000FFFFC
#define POCMR_CM_8K	0x000FFFFE
#define POCMR_CM_4K	0x000FFFFF

/* PITAR - PCI Inbound Translation Address Register
 */
#define PITAR_TA_MASK			0x000fffff

/* PIBAR - PCI Inbound Base/Extended Address Register
 */
#define PIBAR_MASK			0xffffffff
#define PIEBAR_EBA_MASK			0x000fffff

/* PIWAR - PCI Inbound Windows Attributes Register
 */
#define PIWAR_EN			0x80000000
#define PIWAR_SBS			0x40000000
#define PIWAR_PF			0x20000000
#define PIWAR_RTT_MASK			0x000f0000
#define PIWAR_RTT_NO_SNOOP		0x00040000
#define PIWAR_RTT_SNOOP			0x00050000
#define PIWAR_WTT_MASK			0x0000f000
#define PIWAR_WTT_NO_SNOOP		0x00004000
#define PIWAR_WTT_SNOOP			0x00005000
#define PIWAR_IWS_MASK			0x0000003F
#define PIWAR_IWS_4K			0x0000000B
#define PIWAR_IWS_8K			0x0000000C
#define PIWAR_IWS_16K			0x0000000D
#define PIWAR_IWS_32K			0x0000000E
#define PIWAR_IWS_64K			0x0000000F
#define PIWAR_IWS_128K			0x00000010
#define PIWAR_IWS_256K			0x00000011
#define PIWAR_IWS_512K			0x00000012
#define PIWAR_IWS_1M			0x00000013
#define PIWAR_IWS_2M			0x00000014
#define PIWAR_IWS_4M			0x00000015
#define PIWAR_IWS_8M			0x00000016
#define PIWAR_IWS_16M			0x00000017
#define PIWAR_IWS_32M			0x00000018
#define PIWAR_IWS_64M			0x00000019
#define PIWAR_IWS_128M			0x0000001A
#define PIWAR_IWS_256M			0x0000001B
#define PIWAR_IWS_512M			0x0000001C
#define PIWAR_IWS_1G			0x0000001D
#define PIWAR_IWS_2G			0x0000001E

#endif	/* __MPC512X_H__ */

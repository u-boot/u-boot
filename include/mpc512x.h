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
#define SCFR1_IPS_DIV			0x2
#define SCFR1_IPS_DIV_MASK		0x03800000
#define SCFR1_IPS_DIV_SHIFT		23

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

/* Indexes in regs array */
#define MEM_IDX			0x00
#define SPDIF_TXCLOCK_IDX	0x73
#define SPDIF_TX_IDX		0x74
#define SPDIF_RX_IDX		0x75
#define PSC0_0_IDX		0x83
#define PSC0_1_IDX		0x84
#define PSC0_2_IDX		0x85
#define PSC0_3_IDX		0x86
#define PSC0_4_IDX		0x87
#define PSC1_0_IDX		0x88
#define PSC1_1_IDX		0x89
#define PSC1_2_IDX		0x8a
#define PSC1_3_IDX		0x8b
#define PSC1_4_IDX		0x8c
#define PSC2_0_IDX		0x8d
#define PSC2_1_IDX		0x8e
#define PSC2_2_IDX		0x8f
#define PSC2_3_IDX		0x90
#define PSC2_4_IDX		0x91

#define IOCTRL_FUNCMUX_SHIFT	7
#define IOCTRL_FUNCMUX_FEC	1
#define IOCTRL_MUX_FEC		(IOCTRL_FUNCMUX_FEC << IOCTRL_FUNCMUX_SHIFT)

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

#endif	/* __MPC512X_H__ */

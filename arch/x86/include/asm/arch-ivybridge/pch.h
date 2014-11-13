/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot src/southbridge/intel/bd82x6x/pch.h
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2012 The Chromium OS Authors.  All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ASM_ARCH_PCH_H
#define _ASM_ARCH_PCH_H

#include <pci.h>

#define DEFAULT_GPIOBASE	0x0480
#define DEFAULT_PMBASE		0x0500

#define SMBUS_IO_BASE		0x0400

#define PCH_EHCI1_DEV		PCI_BDF(0, 0x1d, 0)
#define PCH_EHCI2_DEV		PCI_BDF(0, 0x1a, 0)
#define PCH_XHCI_DEV		PCI_BDF(0, 0x14, 0)
#define PCH_ME_DEV		PCI_BDF(0, 0x16, 0)
#define PCH_PCIE_DEV_SLOT	28

#define PCH_DEV			PCI_BDF(0, 0, 0)
#define PCH_VIDEO_DEV		PCI_BDF(0, 2, 0)

/* PCI Configuration Space (D31:F0): LPC */
#define PCH_LPC_DEV		PCI_BDF(0, 0x1f, 0)

#define PMBASE			0x40
#define ACPI_CNTL		0x44
#define BIOS_CNTL		0xDC
#define GPIO_BASE		0x48 /* LPC GPIO Base Address Register */
#define GPIO_CNTL		0x4C /* LPC GPIO Control Register */
#define GPIO_ROUT		0xb8

#define LPC_IO_DEC		0x80 /* IO Decode Ranges Register */
#define LPC_EN			0x82 /* LPC IF Enables Register */
#define  CNF2_LPC_EN		(1 << 13) /* 0x4e/0x4f */
#define  CNF1_LPC_EN		(1 << 12) /* 0x2e/0x2f */
#define  MC_LPC_EN		(1 << 11) /* 0x62/0x66 */
#define  KBC_LPC_EN		(1 << 10) /* 0x60/0x64 */
#define  GAMEH_LPC_EN		(1 << 9)  /* 0x208/0x20f */
#define  GAMEL_LPC_EN		(1 << 8)  /* 0x200/0x207 */
#define  FDD_LPC_EN		(1 << 3)  /* LPC_IO_DEC[12] */
#define  LPT_LPC_EN		(1 << 2)  /* LPC_IO_DEC[9:8] */
#define  COMB_LPC_EN		(1 << 1)  /* LPC_IO_DEC[6:4] */
#define  COMA_LPC_EN		(1 << 0)  /* LPC_IO_DEC[3:2] */
#define LPC_GEN1_DEC		0x84 /* LPC IF Generic Decode Range 1 */
#define LPC_GEN2_DEC		0x88 /* LPC IF Generic Decode Range 2 */
#define LPC_GEN3_DEC		0x8c /* LPC IF Generic Decode Range 3 */
#define LPC_GEN4_DEC		0x90 /* LPC IF Generic Decode Range 4 */
#define LPC_GENX_DEC(x)		(0x84 + 4 * (x))

/* PCI Configuration Space (D31:F3): SMBus */
#define PCH_SMBUS_DEV		PCI_BDF(0, 0x1f, 3)
#define SMB_BASE		0x20
#define HOSTC			0x40
#define SMB_RCV_SLVA		0x09

/* HOSTC bits */
#define I2C_EN			(1 << 2)
#define SMB_SMI_EN		(1 << 1)
#define HST_EN			(1 << 0)

/* SMBus I/O bits. */
#define SMBHSTSTAT		0x0
#define SMBHSTCTL		0x2
#define SMBHSTCMD		0x3
#define SMBXMITADD		0x4
#define SMBHSTDAT0		0x5
#define SMBHSTDAT1		0x6
#define SMBBLKDAT		0x7
#define SMBTRNSADD		0x9
#define SMBSLVDATA		0xa
#define SMLINK_PIN_CTL		0xe
#define SMBUS_PIN_CTL		0xf

#define SMBUS_TIMEOUT		(10 * 1000 * 100)


/* Root Complex Register Block */
#define DEFAULT_RCBA		0xfed1c000
#define RCB_REG(reg)		(DEFAULT_RCBA + (reg))

#define PCH_RCBA_BASE		0xf0

#define VCH		0x0000	/* 32bit */
#define VCAP1		0x0004	/* 32bit */
#define VCAP2		0x0008	/* 32bit */
#define PVC		0x000c	/* 16bit */
#define PVS		0x000e	/* 16bit */

#define V0CAP		0x0010	/* 32bit */
#define V0CTL		0x0014	/* 32bit */
#define V0STS		0x001a	/* 16bit */

#define V1CAP		0x001c	/* 32bit */
#define V1CTL		0x0020	/* 32bit */
#define V1STS		0x0026	/* 16bit */

#define RCTCL		0x0100	/* 32bit */
#define ESD		0x0104	/* 32bit */
#define ULD		0x0110	/* 32bit */
#define ULBA		0x0118	/* 64bit */

#define RP1D		0x0120	/* 32bit */
#define RP1BA		0x0128	/* 64bit */
#define RP2D		0x0130	/* 32bit */
#define RP2BA		0x0138	/* 64bit */
#define RP3D		0x0140	/* 32bit */
#define RP3BA		0x0148	/* 64bit */
#define RP4D		0x0150	/* 32bit */
#define RP4BA		0x0158	/* 64bit */
#define HDD		0x0160	/* 32bit */
#define HDBA		0x0168	/* 64bit */
#define RP5D		0x0170	/* 32bit */
#define RP5BA		0x0178	/* 64bit */
#define RP6D		0x0180	/* 32bit */
#define RP6BA		0x0188	/* 64bit */

#define RPC		0x0400	/* 32bit */
#define RPFN		0x0404	/* 32bit */

#define SPI_FREQ_SWSEQ	0x3893
#define SPI_DESC_COMP0	0x38b0
#define SPI_FREQ_WR_ERA	0x38b4
#define SOFT_RESET_CTRL 0x38f4
#define SOFT_RESET_DATA 0x38f8

#define RC		0x3400	/* 32bit */
#define HPTC		0x3404	/* 32bit */
#define GCS		0x3410	/* 32bit */
#define BUC		0x3414	/* 32bit */
#define PCH_DISABLE_GBE		(1 << 5)
#define FD		0x3418	/* 32bit */
#define DISPBDF		0x3424  /* 16bit */
#define FD2		0x3428	/* 32bit */
#define CG		0x341c	/* 32bit */

/* ICH7 PMBASE */
#define PM1_STS		0x00
#define   WAK_STS	(1 << 15)
#define   PCIEXPWAK_STS	(1 << 14)
#define   PRBTNOR_STS	(1 << 11)
#define   RTC_STS	(1 << 10)
#define   PWRBTN_STS	(1 << 8)
#define   GBL_STS	(1 << 5)
#define   BM_STS	(1 << 4)
#define   TMROF_STS	(1 << 0)
#define PM1_EN		0x02
#define   PCIEXPWAK_DIS	(1 << 14)
#define   RTC_EN	(1 << 10)
#define   PWRBTN_EN	(1 << 8)
#define   GBL_EN	(1 << 5)
#define   TMROF_EN	(1 << 0)
#define PM1_CNT		0x04
#define   SLP_EN	(1 << 13)
#define   SLP_TYP	(7 << 10)
#define    SLP_TYP_S0	0
#define    SLP_TYP_S1	1
#define    SLP_TYP_S3	5
#define    SLP_TYP_S4	6
#define    SLP_TYP_S5	7
#define   GBL_RLS	(1 << 2)
#define   BM_RLD	(1 << 1)
#define   SCI_EN	(1 << 0)
#define PM1_TMR		0x08
#define PROC_CNT	0x10
#define LV2		0x14
#define LV3		0x15
#define LV4		0x16
#define PM2_CNT		0x50 /* mobile only */
#define GPE0_STS	0x20
#define   PME_B0_STS	(1 << 13)
#define   PME_STS	(1 << 11)
#define   BATLOW_STS	(1 << 10)
#define   PCI_EXP_STS	(1 << 9)
#define   RI_STS	(1 << 8)
#define   SMB_WAK_STS	(1 << 7)
#define   TCOSCI_STS	(1 << 6)
#define   SWGPE_STS	(1 << 2)
#define   HOT_PLUG_STS	(1 << 1)
#define GPE0_EN		0x28
#define   PME_B0_EN	(1 << 13)
#define   PME_EN	(1 << 11)
#define   TCOSCI_EN	(1 << 6)
#define SMI_EN		0x30
#define   INTEL_USB2_EN	 (1 << 18) /* Intel-Specific USB2 SMI logic */
#define   LEGACY_USB2_EN (1 << 17) /* Legacy USB2 SMI logic */
#define   PERIODIC_EN	 (1 << 14) /* SMI on PERIODIC_STS in SMI_STS */
#define   TCO_EN	 (1 << 13) /* Enable TCO Logic (BIOSWE et al) */
#define   MCSMI_EN	 (1 << 11) /* Trap microcontroller range access */
#define   BIOS_RLS	 (1 <<  7) /* asserts SCI on bit set */
#define   SWSMI_TMR_EN	 (1 <<  6) /* start software smi timer on bit set */
#define   APMC_EN	 (1 <<  5) /* Writes to APM_CNT cause SMI# */
#define   SLP_SMI_EN	 (1 <<  4) /* Write SLP_EN in PM1_CNT asserts SMI# */
#define   LEGACY_USB_EN  (1 <<  3) /* Legacy USB circuit SMI logic */
#define   BIOS_EN	 (1 <<  2) /* Assert SMI# on setting GBL_RLS bit */
#define   EOS		 (1 <<  1) /* End of SMI (deassert SMI#) */
#define   GBL_SMI_EN	 (1 <<  0) /* SMI# generation at all? */
#define SMI_STS		0x34
#define ALT_GP_SMI_EN	0x38
#define ALT_GP_SMI_STS	0x3a
#define GPE_CNTL	0x42
#define DEVACT_STS	0x44
#define SS_CNT		0x50
#define C3_RES		0x54
#define TCO1_STS	0x64
#define   DMISCI_STS	(1 << 9)
#define TCO2_STS	0x66

/**
 * lpc_early_init() - set up LPC serial ports and other early things
 *
 * @blob:	Device tree blob
 * @node:	Offset of LPC node
 * @dev:	PCH PCI device containing the LPC
 * @return 0 if OK, -ve on error
 */
int lpc_early_init(const void *blob, int node, pci_dev_t dev);

#endif

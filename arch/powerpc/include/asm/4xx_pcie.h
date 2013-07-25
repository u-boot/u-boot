/*
 * Copyright (c) 2005 Cisco Systems.  All rights reserved.
 * Roland Dreier <rolandd@cisco.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __4XX_PCIE_H
#define __4XX_PCIE_H

#include <asm/ppc4xx.h>
#include <pci.h>

#define DCRN_SDR0_CFGADDR	0x00e
#define DCRN_SDR0_CFGDATA	0x00f

#if defined(CONFIG_440SPE)
#define CONFIG_SYS_PCIE_NR_PORTS	3

#define CONFIG_SYS_PCIE_ADDR_HIGH	0x0000000d

#define DCRN_PCIE0_BASE		0x100
#define DCRN_PCIE1_BASE		0x120
#define DCRN_PCIE2_BASE		0x140

#define PCIE0_SDR		0x300
#define PCIE1_SDR		0x340
#define PCIE2_SDR		0x370
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define CONFIG_SYS_PCIE_NR_PORTS	2

#define CONFIG_SYS_PCIE_ADDR_HIGH	0x0000000d

#define DCRN_PCIE0_BASE		0x100
#define DCRN_PCIE1_BASE		0x120

#define PCIE0_SDR		0x300
#define PCIE1_SDR		0x340
#endif

#if defined(CONFIG_405EX)
#define CONFIG_SYS_PCIE_NR_PORTS	2

#define CONFIG_SYS_PCIE_ADDR_HIGH	0x00000000

#define	DCRN_PCIE0_BASE		0x040
#define	DCRN_PCIE1_BASE		0x060

#define PCIE0_SDR		0x400
#define PCIE1_SDR		0x440
#endif

#define PCIE0			DCRN_PCIE0_BASE
#define PCIE1			DCRN_PCIE1_BASE
#define PCIE2			DCRN_PCIE2_BASE

#define DCRN_PEGPL_CFGBAH(base)		(base + 0x00)
#define DCRN_PEGPL_CFGBAL(base)		(base + 0x01)
#define DCRN_PEGPL_CFGMSK(base)		(base + 0x02)
#define DCRN_PEGPL_MSGBAH(base)		(base + 0x03)
#define DCRN_PEGPL_MSGBAL(base)		(base + 0x04)
#define DCRN_PEGPL_MSGMSK(base)		(base + 0x05)
#define DCRN_PEGPL_OMR1BAH(base)	(base + 0x06)
#define DCRN_PEGPL_OMR1BAL(base)	(base + 0x07)
#define DCRN_PEGPL_OMR1MSKH(base)	(base + 0x08)
#define DCRN_PEGPL_OMR1MSKL(base)	(base + 0x09)
#define DCRN_PEGPL_REGBAH(base)		(base + 0x12)
#define DCRN_PEGPL_REGBAL(base)		(base + 0x13)
#define DCRN_PEGPL_REGMSK(base)		(base + 0x14)
#define DCRN_PEGPL_SPECIAL(base)	(base + 0x15)
#define DCRN_PEGPL_CFG(base)		(base + 0x16)

/*
 * System DCRs (SDRs)
 */
#define PESDR0_PLLLCT1		0x03a0
#define PESDR0_PLLLCT2		0x03a1
#define PESDR0_PLLLCT3		0x03a2

/* common regs, at for all 4xx with PCIe core */
#define SDRN_PESDR_UTLSET1(n)		(sdr_base(n) + 0x00)
#define SDRN_PESDR_UTLSET2(n)		(sdr_base(n) + 0x01)
#define SDRN_PESDR_DLPSET(n)		(sdr_base(n) + 0x02)
#define SDRN_PESDR_LOOP(n)		(sdr_base(n) + 0x03)
#define SDRN_PESDR_RCSSET(n)		(sdr_base(n) + 0x04)
#define SDRN_PESDR_RCSSTS(n)		(sdr_base(n) + 0x05)

#if defined(CONFIG_440SPE)
#define SDRN_PESDR_HSSL0SET1(n)		(sdr_base(n) + 0x06)
#define SDRN_PESDR_HSSL0SET2(n)		(sdr_base(n) + 0x07)
#define SDRN_PESDR_HSSL0STS(n)		(sdr_base(n) + 0x08)
#define SDRN_PESDR_HSSL1SET1(n)		(sdr_base(n) + 0x09)
#define SDRN_PESDR_HSSL1SET2(n)		(sdr_base(n) + 0x0a)
#define SDRN_PESDR_HSSL1STS(n)		(sdr_base(n) + 0x0b)
#define SDRN_PESDR_HSSL2SET1(n)		(sdr_base(n) + 0x0c)
#define SDRN_PESDR_HSSL2SET2(n)		(sdr_base(n) + 0x0d)
#define SDRN_PESDR_HSSL2STS(n)		(sdr_base(n) + 0x0e)
#define SDRN_PESDR_HSSL3SET1(n)		(sdr_base(n) + 0x0f)
#define SDRN_PESDR_HSSL3SET2(n)		(sdr_base(n) + 0x10)
#define SDRN_PESDR_HSSL3STS(n)		(sdr_base(n) + 0x11)

#define PESDR0_UTLSET1		0x0300
#define PESDR0_UTLSET2		0x0301
#define PESDR0_DLPSET		0x0302
#define PESDR0_LOOP		0x0303
#define PESDR0_RCSSET		0x0304
#define PESDR0_RCSSTS		0x0305
#define PESDR0_HSSL0SET1	0x0306
#define PESDR0_HSSL0SET2	0x0307
#define PESDR0_HSSL0STS		0x0308
#define PESDR0_HSSL1SET1	0x0309
#define PESDR0_HSSL1SET2	0x030a
#define PESDR0_HSSL1STS		0x030b
#define PESDR0_HSSL2SET1	0x030c
#define PESDR0_HSSL2SET2	0x030d
#define PESDR0_HSSL2STS		0x030e
#define PESDR0_HSSL3SET1	0x030f
#define PESDR0_HSSL3SET2	0x0310
#define PESDR0_HSSL3STS		0x0311
#define PESDR0_HSSL4SET1	0x0312
#define PESDR0_HSSL4SET2	0x0313
#define PESDR0_HSSL4STS		0x0314
#define PESDR0_HSSL5SET1	0x0315
#define PESDR0_HSSL5SET2	0x0316
#define PESDR0_HSSL5STS		0x0317
#define PESDR0_HSSL6SET1	0x0318
#define PESDR0_HSSL6SET2	0x0319
#define PESDR0_HSSL6STS		0x031a
#define PESDR0_HSSL7SET1	0x031b
#define PESDR0_HSSL7SET2	0x031c
#define PESDR0_HSSL7STS		0x031d
#define PESDR0_HSSCTLSET	0x031e
#define PESDR0_LANE_ABCD	0x031f
#define PESDR0_LANE_EFGH	0x0320

#define PESDR1_UTLSET1		0x0340
#define PESDR1_UTLSET2		0x0341
#define PESDR1_DLPSET		0x0342
#define PESDR1_LOOP		0x0343
#define PESDR1_RCSSET		0x0344
#define PESDR1_RCSSTS		0x0345
#define PESDR1_HSSL0SET1	0x0346
#define PESDR1_HSSL0SET2	0x0347
#define PESDR1_HSSL0STS		0x0348
#define PESDR1_HSSL1SET1	0x0349
#define PESDR1_HSSL1SET2	0x034a
#define PESDR1_HSSL1STS		0x034b
#define PESDR1_HSSL2SET1	0x034c
#define PESDR1_HSSL2SET2	0x034d
#define PESDR1_HSSL2STS		0x034e
#define PESDR1_HSSL3SET1	0x034f
#define PESDR1_HSSL3SET2	0x0350
#define PESDR1_HSSL3STS		0x0351
#define PESDR1_HSSCTLSET	0x0352
#define PESDR1_LANE_ABCD	0x0353

#define PESDR2_UTLSET1		0x0370
#define PESDR2_UTLSET2		0x0371
#define PESDR2_DLPSET		0x0372
#define PESDR2_LOOP		0x0373
#define PESDR2_RCSSET		0x0374
#define PESDR2_RCSSTS		0x0375
#define PESDR2_HSSL0SET1	0x0376
#define PESDR2_HSSL0SET2	0x0377
#define PESDR2_HSSL0STS		0x0378
#define PESDR2_HSSL1SET1	0x0379
#define PESDR2_HSSL1SET2	0x037a
#define PESDR2_HSSL1STS		0x037b
#define PESDR2_HSSL2SET1	0x037c
#define PESDR2_HSSL2SET2	0x037d
#define PESDR2_HSSL2STS		0x037e
#define PESDR2_HSSL3SET1	0x037f
#define PESDR2_HSSL3SET2	0x0380
#define PESDR2_HSSL3STS		0x0381
#define PESDR2_HSSCTLSET	0x0382
#define PESDR2_LANE_ABCD	0x0383

#elif defined(CONFIG_405EX)

#define SDRN_PESDR_PHYSET1(n)		(sdr_base(n) + 0x06)
#define SDRN_PESDR_PHYSET2(n)		(sdr_base(n) + 0x07)
#define SDRN_PESDR_BIST(n)		(sdr_base(n) + 0x08)
#define SDRN_PESDR_LPB(n)		(sdr_base(n) + 0x0b)
#define SDRN_PESDR_PHYSTA(n)		(sdr_base(n) + 0x0c)

#define PESDR0_UTLSET1		0x0400
#define PESDR0_UTLSET2		0x0401
#define PESDR0_DLPSET		0x0402
#define PESDR0_LOOP		0x0403
#define PESDR0_RCSSET		0x0404
#define PESDR0_RCSSTS		0x0405
#define PESDR0_PHYSET1		0x0406
#define PESDR0_PHYSET2		0x0407
#define PESDR0_BIST		0x0408
#define PESDR0_LPB		0x040B
#define PESDR0_PHYSTA		0x040C

#define PESDR1_UTLSET1		0x0440
#define PESDR1_UTLSET2		0x0441
#define PESDR1_DLPSET		0x0442
#define PESDR1_LOOP		0x0443
#define PESDR1_RCSSET		0x0444
#define PESDR1_RCSSTS		0x0445
#define PESDR1_PHYSET1		0x0446
#define PESDR1_PHYSET2		0x0447
#define PESDR1_BIST		0x0448
#define PESDR1_LPB		0x044B
#define PESDR1_PHYSTA		0x044C

#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)

#define PESDR0_L0BIST		0x0308	/* PE0 L0 built in self test */
#define PESDR0_L0BISTSTS	0x0309	/* PE0 L0 built in self test status */
#define PESDR0_L0CDRCTL		0x030A	/* PE0 L0 CDR control */
#define PESDR0_L0DRV		0x030B	/* PE0 L0 drive */
#define PESDR0_L0REC		0x030C	/* PE0 L0 receiver */
#define PESDR0_L0LPB		0x030D	/* PE0 L0 loopback */
#define PESDR0_L0CLK		0x030E	/* PE0 L0 clocking */
#define PESDR0_PHY_CTL_RST	0x030F	/* PE0 PHY control reset */
#define PESDR0_RSTSTA		0x0310	/* PE0 reset status */
#define PESDR0_OBS		0x0311	/* PE0 observation register */
#define PESDR0_L0ERRC		0x0320	/* PE0 L0 error counter */

#define PESDR1_L0BIST		0x0348	/* PE1 L0 built in self test */
#define PESDR1_L1BIST		0x0349	/* PE1 L1 built in self test */
#define PESDR1_L2BIST		0x034A	/* PE1 L2 built in self test */
#define PESDR1_L3BIST		0x034B	/* PE1 L3 built in self test */
#define PESDR1_L0BISTSTS	0x034C	/* PE1 L0 built in self test status */
#define PESDR1_L1BISTSTS	0x034D	/* PE1 L1 built in self test status */
#define PESDR1_L2BISTSTS	0x034E	/* PE1 L2 built in self test status */
#define PESDR1_L3BISTSTS	0x034F	/* PE1 L3 built in self test status */
#define PESDR1_L0CDRCTL		0x0350	/* PE1 L0 CDR control */
#define PESDR1_L1CDRCTL		0x0351	/* PE1 L1 CDR control */
#define PESDR1_L2CDRCTL		0x0352	/* PE1 L2 CDR control */
#define PESDR1_L3CDRCTL		0x0353	/* PE1 L3 CDR control */
#define PESDR1_L0DRV		0x0354	/* PE1 L0 drive */
#define PESDR1_L1DRV		0x0355	/* PE1 L1 drive */
#define PESDR1_L2DRV		0x0356	/* PE1 L2 drive */
#define PESDR1_L3DRV		0x0357	/* PE1 L3 drive */
#define PESDR1_L0REC		0x0358	/* PE1 L0 receiver */
#define PESDR1_L1REC		0x0359	/* PE1 L1 receiver */
#define PESDR1_L2REC		0x035A	/* PE1 L2 receiver */
#define PESDR1_L3REC		0x035B	/* PE1 L3 receiver */
#define PESDR1_L0LPB		0x035C	/* PE1 L0 loopback */
#define PESDR1_L1LPB		0x035D	/* PE1 L1 loopback */
#define PESDR1_L2LPB		0x035E	/* PE1 L2 loopback */
#define PESDR1_L3LPB		0x035F	/* PE1 L3 loopback */
#define PESDR1_L0CLK		0x0360	/* PE1 L0 clocking */
#define PESDR1_L1CLK		0x0361	/* PE1 L1 clocking */
#define PESDR1_L2CLK		0x0362	/* PE1 L2 clocking */
#define PESDR1_L3CLK		0x0363	/* PE1 L3 clocking */
#define PESDR1_PHY_CTL_RST	0x0364	/* PE1 PHY control reset */
#define PESDR1_RSTSTA		0x0365	/* PE1 reset status */
#define PESDR1_OBS		0x0366	/* PE1 observation register */
#define PESDR1_L0ERRC		0x0368	/* PE1 L0 error counter */
#define PESDR1_L1ERRC		0x0369	/* PE1 L1 error counter */
#define PESDR1_L2ERRC		0x036A	/* PE1 L2 error counter */
#define PESDR1_L3ERRC		0x036B	/* PE1 L3 error counter */
#define PESDR0_IHS1		0x036C	/* PE interrupt handler interfact setting 1 */
#define PESDR0_IHS2		0x036D	/* PE interrupt handler interfact setting 2 */

#endif

/* SDR Bit Mappings */
#define PESDRx_RCSSET_HLDPLB	0x10000000
#define PESDRx_RCSSET_RSTGU	0x01000000
#define PESDRx_RCSSET_RDY       0x00100000
#define PESDRx_RCSSET_RSTDL     0x00010000
#define PESDRx_RCSSET_RSTPYN    0x00001000

#define PESDRx_RCSSTS_PLBIDL	0x10000000
#define PESDRx_RCSSTS_HRSTRQ	0x01000000
#define PESDRx_RCSSTS_PGRST	0x00100000
#define PESDRx_RCSSTS_VC0ACT	0x00010000
#define PESDRx_RCSSTS_BMEN	0x00000100

/*
 * UTL register offsets
 */
#define	PEUTL_PBCTL		0x00
#define PEUTL_PBBSZ		0x20
#define PEUTL_OPDBSZ		0x68
#define PEUTL_IPHBSZ		0x70
#define PEUTL_IPDBSZ		0x78
#define PEUTL_OUTTR		0x90
#define PEUTL_INTR		0x98
#define PEUTL_PCTL		0xa0
#define	PEUTL_RCSTA		0xb0
#define PEUTL_RCIRQEN		0xb8

/*
 * Config space register offsets
 */
#define PECFG_BAR0LMPA		0x210
#define PECFG_BAR0HMPA		0x214
#define PECFG_BAR1MPA		0x218
#define PECFG_BAR2LMPA		0x220
#define PECFG_BAR2HMPA		0x224

#define PECFG_PIMEN		0x33c
#define PECFG_PIM0LAL		0x340
#define PECFG_PIM0LAH		0x344
#define PECFG_PIM1LAL		0x348
#define PECFG_PIM1LAH		0x34c
#define PECFG_PIM01SAL		0x350
#define PECFG_PIM01SAH		0x354

#define PECFG_POM0LAL		0x380
#define PECFG_POM0LAH		0x384

#define SDR_READ(offset) ({\
	mtdcr(DCRN_SDR0_CFGADDR, offset); \
	mfdcr(DCRN_SDR0_CFGDATA);})

#define SDR_WRITE(offset, data) ({\
	mtdcr(DCRN_SDR0_CFGADDR, offset); \
	mtdcr(DCRN_SDR0_CFGDATA,data);})

#define GPL_DMER_MASK_DISA	0x02000000

#define U64_TO_U32_LOW(val)	((u32)((val) & 0x00000000ffffffffULL))
#define U64_TO_U32_HIGH(val)	((u32)((val) >> 32))

/*
 * Prototypes
 */
int ppc4xx_init_pcie(void);
int ppc4xx_init_pcie_rootport(int port);
int ppc4xx_init_pcie_endport(int port);
void ppc4xx_setup_pcie_rootpoint(struct pci_controller *hose, int port);
int ppc4xx_setup_pcie_endpoint(struct pci_controller *hose, int port);
int pcie_hose_scan(struct pci_controller *hose, int bus);

/*
 * Function to determine root port or endport from env variable.
 */
static inline int is_end_point(int port)
{
	char s[10], *tk;
	char *pcie_mode = getenv("pcie_mode");

	if (pcie_mode == NULL)
		return 0;

	strcpy(s, pcie_mode);
	tk = strtok(s, ":");

	switch (port) {
	case 0:
		if (tk != NULL) {
			if (!(strcmp(tk, "ep") && strcmp(tk, "EP")))
				return 1;
			else
				return 0;
		}
		else
			return 0;

	case 1:
		tk = strtok(NULL, ":");
		if (tk != NULL) {
			if (!(strcmp(tk, "ep") && strcmp(tk, "EP")))
				return 1;
			else
				return 0;
		}
		else
			return 0;

	case 2:
		tk = strtok(NULL, ":");
		if (tk != NULL)
			tk = strtok(NULL, ":");
		if (tk != NULL) {
			if (!(strcmp(tk, "ep") && strcmp(tk, "EP")))
				return 1;
			else
				return 0;
		}
		else
			return 0;
	}

	return 0;
}

#if defined(PCIE0_SDR)
static inline u32 sdr_base(int port)
{
	switch (port) {
	default:	/* to satisfy compiler */
	case 0:
		return PCIE0_SDR;
	case 1:
		return PCIE1_SDR;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	case 2:
		return PCIE2_SDR;
#endif
	}
}
#endif /* defined(PCIE0_SDR) */

#endif /* __4XX_PCIE_H */

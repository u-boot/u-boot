/*
 * UniPhier UMC (Universal Memory Controller) registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_UMC_REGS_H
#define ARCH_UMC_REGS_H

#define UMC_BASE		0x5b800000

/* SSIF registers */
#define UMC_SSIF_BASE		UMC_BASE

#define UMC_CPURST		0x00000700
#define UMC_IDSRST		0x0000070C
#define UMC_IXMRST		0x00000714
#define UMC_HDMRST		0x00000718
#define UMC_MDMRST		0x0000071C
#define UMC_HDDRST		0x00000720
#define UMC_MDDRST		0x00000724
#define UMC_SIORST		0x00000728
#define UMC_GIORST		0x0000072C
#define UMC_HD2RST		0x00000734
#define UMC_VIORST		0x0000073C
#define UMC_FRCRST		0x00000748 /* LD4/sLD8 */
#define UMC_DVCRST		0x00000748 /* Pro4 */
#define UMC_RGLRST		0x00000750
#define UMC_VPERST		0x00000758
#define UMC_AIORST		0x00000764
#define UMC_DMDRST		0x00000770

#define UMC_HDMCHSEL		0x00000898
#define UMC_MDMCHSEL		0x0000089C
#define UMC_DVCCHSEL		0x000008C8
#define UMC_DMDCHSEL		0x000008F0

#define UMC_CLKEN_SSIF_FETCH	0x0000C060
#define UMC_CLKEN_SSIF_COMQUE0	0x0000C064
#define UMC_CLKEN_SSIF_COMWC0	0x0000C068
#define UMC_CLKEN_SSIF_COMRC0	0x0000C06C
#define UMC_CLKEN_SSIF_COMQUE1	0x0000C070
#define UMC_CLKEN_SSIF_COMWC1	0x0000C074
#define UMC_CLKEN_SSIF_COMRC1	0x0000C078
#define UMC_CLKEN_SSIF_WC	0x0000C07C
#define UMC_CLKEN_SSIF_RC	0x0000C080
#define UMC_CLKEN_SSIF_DST	0x0000C084

/* CA registers */
#define UMC_CA_BASE(ch)		(UMC_BASE + 0x00001000 + 0x00001000 * (ch))

/* DRAM controller registers */
#define UMC_DRAMCONT_BASE(ch)	(UMC_BASE + 0x00400000 + 0x00200000 * (ch))

#define UMC_CMDCTLA		0x00000000
#define UMC_CMDCTLB		0x00000004
#define UMC_INITCTLA		0x00000008
#define UMC_INITCTLB		0x0000000C
#define UMC_INITCTLC		0x00000010
#define UMC_INITSET		0x00000014
#define UMC_INITSTAT		0x00000018
#define UMC_DRMMR0		0x0000001C
#define UMC_DRMMR1		0x00000020
#define UMC_DRMMR2		0x00000024
#define UMC_DRMMR3		0x00000028
#define UMC_SPCCTLA		0x00000030
#define UMC_SPCCTLB		0x00000034
#define UMC_SPCSETA		0x00000038
#define UMC_SPCSETB		0x0000003C
#define UMC_SPCSETC		0x00000040
#define UMC_SPCSETD		0x00000044
#define UMC_SPCSTATA		0x00000050
#define UMC_SPCSTATB		0x00000054
#define UMC_SPCSTATC		0x00000058
#define UMC_ACSSETA		0x00000060
#define UMC_FLOWCTLA		0x00000400
#define UMC_FLOWCTLB		0x00000404
#define UMC_FLOWCTLC		0x00000408
#define UMC_FLOWCTLG		0x00000508
#define UMC_RDATACTL_D0		0x00000600
#define UMC_WDATACTL_D0		0x00000604
#define UMC_RDATACTL_D1		0x00000608
#define UMC_WDATACTL_D1		0x0000060C
#define UMC_DATASET		0x00000610
#define UMC_DCCGCTL		0x00000720
#define UMC_DICGCTLA		0x00000724
#define UMC_DICGCTLB		0x00000728
#define UMC_DIOCTLA		0x00000C00
#define UMC_DFICUPDCTLA		0x00000C20

#ifndef __ASSEMBLY__

#include <linux/types.h>

static inline void umc_polling(u32 address, u32 expval, u32 mask)
{
	u32 nmask = ~mask;
	u32 data;
	do {
		data = readl(address) & nmask;
	} while (data != expval);
}

static inline void umc_dram_init_start(void __iomem *dramcont)
{
	writel(0x00000002, dramcont + UMC_INITSET);
}

static inline void umc_dram_init_poll(void __iomem *dramcont)
{
	while ((readl(dramcont + UMC_INITSTAT) & 0x00000002))
		;
}

#endif

#endif

/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/fsl_law.h>
#include <asm/errno.h>
#include "fsl_corenet2_serdes.h"

#ifdef CONFIG_SYS_FSL_SRDS_1
static u64 serdes1_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u64 serdes2_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
static u64 serdes3_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
static u64 serdes4_prtcl_map;
#endif

#ifdef DEBUG
static const char *serdes_prtcl_str[] = {
	[NONE] = "NA",
	[PCIE1] = "PCIE1",
	[PCIE2] = "PCIE2",
	[PCIE3] = "PCIE3",
	[PCIE4] = "PCIE4",
	[SATA1] = "SATA1",
	[SATA2] = "SATA2",
	[SRIO1] = "SRIO1",
	[SRIO2] = "SRIO2",
	[SGMII_FM1_DTSEC1] = "SGMII_FM1_DTSEC1",
	[SGMII_FM1_DTSEC2] = "SGMII_FM1_DTSEC2",
	[SGMII_FM1_DTSEC3] = "SGMII_FM1_DTSEC3",
	[SGMII_FM1_DTSEC4] = "SGMII_FM1_DTSEC4",
	[SGMII_FM1_DTSEC5] = "SGMII_FM1_DTSEC5",
	[SGMII_FM1_DTSEC6] = "SGMII_FM1_DTSEC6",
	[SGMII_FM2_DTSEC1] = "SGMII_FM2_DTSEC1",
	[SGMII_FM2_DTSEC2] = "SGMII_FM2_DTSEC2",
	[SGMII_FM2_DTSEC3] = "SGMII_FM2_DTSEC3",
	[SGMII_FM2_DTSEC4] = "SGMII_FM2_DTSEC4",
	[XAUI_FM1] = "XAUI_FM1",
	[XAUI_FM2] = "XAUI_FM2",
	[AURORA] = "DEBUG",
	[CPRI1] = "CPRI1",
	[CPRI2] = "CPRI2",
	[CPRI3] = "CPRI3",
	[CPRI4] = "CPRI4",
	[CPRI5] = "CPRI5",
	[CPRI6] = "CPRI6",
	[CPRI7] = "CPRI7",
	[CPRI8] = "CPRI8",
	[XAUI_FM1_MAC9] = "XAUI_FM1_MAC9",
	[XAUI_FM1_MAC10] = "XAUI_FM1_MAC10",
	[XAUI_FM2_MAC9] = "XAUI_FM2_MAC9",
	[XAUI_FM2_MAC10] = "XAUI_FM2_MAC10",
	[HIGIG_FM1_MAC9] = "HiGig_FM1_MAC9",
	[HIGIG_FM1_MAC10] = "HiGig_FM1_MAC10",
	[HIGIG_FM2_MAC9] = "HiGig_FM2_MAC9",
	[HIGIG_FM2_MAC10] = "HiGig_FM2_MAC10",
	[QSGMII_FM1_A] = "QSGMII_FM1_A",
	[QSGMII_FM1_B] = "QSGMII_FM1_B",
	[QSGMII_FM2_A] = "QSGMII_FM2_A",
	[QSGMII_FM2_B] = "QSGMII_FM2_B",
	[XFI_FM1_MAC9] = "XFI_FM1_MAC9",
	[XFI_FM1_MAC10] = "XFI_FM1_MAC10",
	[XFI_FM2_MAC9] = "XFI_FM2_MAC9",
	[XFI_FM2_MAC10] = "XFI_FM2_MAC10",
	[INTERLAKEN] = "INTERLAKEN",
	[QSGMII_SW1_A] = "QSGMII_SW1_A",
	[QSGMII_SW1_B] = "QSGMII_SW1_B",
};
#endif

int is_serdes_configured(enum srds_prtcl device)
{
	u64 ret = 0;

#ifdef CONFIG_SYS_FSL_SRDS_1
	ret |= (1ULL << device) & serdes1_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	ret |= (1ULL << device) & serdes2_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
	ret |= (1ULL << device) & serdes3_prtcl_map;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	ret |= (1ULL << device) & serdes4_prtcl_map;
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	const ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 cfg = in_be32(&gur->rcwsr[4]);
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg &= FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg &= FSL_CORENET2_RCWSR4_SRDS2_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
	case FSL_SRDS_3:
		cfg &= FSL_CORENET2_RCWSR4_SRDS3_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	case FSL_SRDS_4:
		cfg &= FSL_CORENET2_RCWSR4_SRDS4_PRTCL;
		cfg >>= FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT;
		break;
#endif
	default:
		printf("invalid SerDes%d\n", sd);
		break;
	}
	/* Is serdes enabled at all? */
	if (unlikely(cfg == 0))
		return -ENODEV;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(sd, cfg, i) == device)
			return i;
	}

	return -ENODEV;
}

u64 serdes_init(u32 sd, u32 sd_addr, u32 sd_prctl_mask, u32 sd_prctl_shift)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u64 serdes_prtcl_map = 0;
	u32 cfg;
	int lane;

	cfg = in_be32(&gur->rcwsr[4]) & sd_prctl_mask;
	/* Is serdes enabled at all? */
	if (!cfg) {
		printf("SERDES%d is not enabled\n", sd + 1);
		return 0;
	}

	cfg >>= sd_prctl_shift;
	printf("Using SERDES%d Protocol: %d (0x%x)\n", sd + 1, cfg, cfg);
	if (!is_serdes_prtcl_valid(sd, cfg))
		printf("SERDES%d[PRTCL] = 0x%x is not valid\n", sd + 1, cfg);

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(sd, cfg, lane);
		serdes_prtcl_map |= (1ULL << lane_prtcl);
	}

	return serdes_prtcl_map;
}

void fsl_serdes_init(void)
{

#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes1_prtcl_map = serdes_init(FSL_SRDS_1,
		CONFIG_SYS_FSL_CORENET_SERDES_ADDR,
		FSL_CORENET2_RCWSR4_SRDS1_PRTCL,
		FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes2_prtcl_map = serdes_init(FSL_SRDS_2,
		CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_2 * 0x1000,
		FSL_CORENET2_RCWSR4_SRDS2_PRTCL,
		FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_3
	serdes3_prtcl_map = serdes_init(FSL_SRDS_3,
		CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_3 * 0x1000,
		FSL_CORENET2_RCWSR4_SRDS3_PRTCL,
		FSL_CORENET2_RCWSR4_SRDS3_PRTCL_SHIFT);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_4
	serdes4_prtcl_map = serdes_init(FSL_SRDS_4,
		CONFIG_SYS_FSL_CORENET_SERDES_ADDR + FSL_SRDS_4 * 0x1000,
		FSL_CORENET2_RCWSR4_SRDS4_PRTCL,
		FSL_CORENET2_RCWSR4_SRDS4_PRTCL_SHIFT);
#endif

}

const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	case SRDS_PLLCR0_RFCK_SEL_156_25:
		return "156.25";
	case SRDS_PLLCR0_RFCK_SEL_161_13:
		return "161.1328123";
	default:
#if defined(CONFIG_T4240QDS)
		return "???";
#else
		return "122.88";
#endif
	}
}


/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch-fsl-lsch3/immap_lsch3.h>
#include <fsl-mc/ldpaa_wriop.h>

#ifdef CONFIG_SYS_FSL_SRDS_1
static u8 serdes1_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u8 serdes2_prtcl_map[SERDES_PRCTL_COUNT];
#endif

int is_serdes_configured(enum srds_prtcl device)
{
	int ret = 0;

#ifdef CONFIG_SYS_FSL_SRDS_1
	ret |= serdes1_prtcl_map[device];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	ret |= serdes2_prtcl_map[device];
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = in_le32(&gur->rcwsr[28]);
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg &= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg &= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;
		break;
#endif
	default:
		printf("invalid SerDes%d\n", sd);
		break;
	}
	/* Is serdes enabled at all? */
	if (cfg == 0)
		return -ENODEV;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(sd, cfg, i) == device)
			return i;
	}

	return -ENODEV;
}

void serdes_init(u32 sd, u32 sd_addr, u32 sd_prctl_mask, u32 sd_prctl_shift,
		u8 serdes_prtcl_map[SERDES_PRCTL_COUNT])
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg;
	int lane;

	memset(serdes_prtcl_map, 0, sizeof(serdes_prtcl_map));

	cfg = in_le32(&gur->rcwsr[28]) & sd_prctl_mask;
	cfg >>= sd_prctl_shift;
	printf("Using SERDES%d Protocol: %d (0x%x)\n", sd + 1, cfg, cfg);

	if (!is_serdes_prtcl_valid(sd, cfg))
		printf("SERDES%d[PRTCL] = 0x%x is not valid\n", sd + 1, cfg);

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(sd, cfg, lane);
		if (unlikely(lane_prtcl >= SERDES_PRCTL_COUNT))
			debug("Unknown SerDes lane protocol %d\n", lane_prtcl);
		else {
			serdes_prtcl_map[lane_prtcl] = 1;
#ifdef CONFIG_FSL_MC_ENET
			wriop_init_dpmac(sd, lane + 1, (int)lane_prtcl);
#endif
		}
	}
}

void fsl_serdes_init(void)
{
#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes_init(FSL_SRDS_1,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR,
		    FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK,
		    FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT,
		    serdes1_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes_init(FSL_SRDS_2,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + FSL_SRDS_2 * 0x10000,
		    FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK,
		    FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT,
		    serdes2_prtcl_map);
#endif
}

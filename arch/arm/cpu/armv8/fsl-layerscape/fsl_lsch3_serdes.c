/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <fsl-mc/ldpaa_wriop.h>

#ifdef CONFIG_SYS_FSL_SRDS_1
static u8 serdes1_prtcl_map[SERDES_PRCTL_COUNT];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
static u8 serdes2_prtcl_map[SERDES_PRCTL_COUNT];
#endif

#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
int xfi_dpmac[XFI8 + 1];
int sgmii_dpmac[SGMII16 + 1];
#endif

__weak void wriop_init_dpmac_qsgmii(int sd, int lane_prtcl)
{
	return;
}

int is_serdes_configured(enum srds_prtcl device)
{
	int ret = 0;

#ifdef CONFIG_SYS_FSL_SRDS_1
	if (!serdes1_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes1_prtcl_map[device];
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	if (!serdes2_prtcl_map[NONE])
		fsl_serdes_init();

	ret |= serdes2_prtcl_map[device];
#endif

	return !!ret;
}

int serdes_get_first_lane(u32 sd, enum srds_prtcl device)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg = 0;
	int i;

	switch (sd) {
#ifdef CONFIG_SYS_FSL_SRDS_1
	case FSL_SRDS_1:
		cfg = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS1_REGSR - 1]);
		cfg &= FSL_CHASSIS3_SRDS1_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_SRDS1_PRTCL_SHIFT;
		break;
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	case FSL_SRDS_2:
		cfg = gur_in32(&gur->rcwsr[FSL_CHASSIS3_SRDS2_REGSR - 1]);
		cfg &= FSL_CHASSIS3_SRDS2_PRTCL_MASK;
		cfg >>= FSL_CHASSIS3_SRDS2_PRTCL_SHIFT;
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

void serdes_init(u32 sd, u32 sd_addr, u32 rcwsr, u32 sd_prctl_mask,
		 u32 sd_prctl_shift, u8 serdes_prtcl_map[SERDES_PRCTL_COUNT])
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 cfg;
	int lane;

	if (serdes_prtcl_map[NONE])
		return;

	memset(serdes_prtcl_map, 0, sizeof(u8) * SERDES_PRCTL_COUNT);

	cfg = gur_in32(&gur->rcwsr[rcwsr - 1]) & sd_prctl_mask;
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
#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
			switch (lane_prtcl) {
			case QSGMII_A:
			case QSGMII_B:
			case QSGMII_C:
			case QSGMII_D:
				wriop_init_dpmac_qsgmii(sd, (int)lane_prtcl);
				break;
			default:
				if (lane_prtcl >= XFI1 && lane_prtcl <= XFI8)
					wriop_init_dpmac(sd,
							 xfi_dpmac[lane_prtcl],
							 (int)lane_prtcl);

				 if (lane_prtcl >= SGMII1 &&
				     lane_prtcl <= SGMII16)
					wriop_init_dpmac(sd, sgmii_dpmac[
							 lane_prtcl],
							 (int)lane_prtcl);
				break;
			}
#endif
		}
	}

	/* Set the first element to indicate serdes has been initialized */
	serdes_prtcl_map[NONE] = 1;
}

void fsl_serdes_init(void)
{
#if defined(CONFIG_FSL_MC_ENET) && !defined(CONFIG_SPL_BUILD)
	int i , j;

	for (i = XFI1, j = 1; i <= XFI8; i++, j++)
		xfi_dpmac[i] = j;

	for (i = SGMII1, j = 1; i <= SGMII16; i++, j++)
		sgmii_dpmac[i] = j;
#endif

#ifdef CONFIG_SYS_FSL_SRDS_1
	serdes_init(FSL_SRDS_1,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR,
		    FSL_CHASSIS3_SRDS1_REGSR,
		    FSL_CHASSIS3_SRDS1_PRTCL_MASK,
		    FSL_CHASSIS3_SRDS1_PRTCL_SHIFT,
		    serdes1_prtcl_map);
#endif
#ifdef CONFIG_SYS_FSL_SRDS_2
	serdes_init(FSL_SRDS_2,
		    CONFIG_SYS_FSL_LSCH3_SERDES_ADDR + FSL_SRDS_2 * 0x10000,
		    FSL_CHASSIS3_SRDS2_REGSR,
		    FSL_CHASSIS3_SRDS2_PRTCL_MASK,
		    FSL_CHASSIS3_SRDS2_PRTCL_SHIFT,
		    serdes2_prtcl_map);
#endif
}

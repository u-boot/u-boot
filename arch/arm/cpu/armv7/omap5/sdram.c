/*
 * Timing and Organization details of the ddr device parts used in OMAP5
 * EVM
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 * Sricharan R <r.sricharan@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/emif.h>
#include <asm/arch/sys_proto.h>

/*
 * This file provides details of the LPDDR2 SDRAM parts used on OMAP5
 * EVM. Since the parts used and geometry are identical for
 * evm for a given OMAP5 revision, this information is kept
 * here instead of being in board directory. However the key functions
 * exported are weakly linked so that they can be over-ridden in the board
 * directory if there is a OMAP5 board in the future that uses a different
 * memory device or geometry.
 *
 * For any new board with different memory devices over-ride one or more
 * of the following functions as per the CONFIG flags you intend to enable:
 * - emif_get_reg_dump()
 * - emif_get_dmm_regs()
 * - emif_get_device_details()
 * - emif_get_device_timings()
 */

#ifdef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
const struct emif_regs emif_regs_532_mhz_2cs = {
	.sdram_config_init		= 0x80800EBA,
	.sdram_config			= 0x808022BA,
	.ref_ctrl			= 0x0000081A,
	.sdram_tim1			= 0x772F6873,
	.sdram_tim2			= 0x304a129a,
	.sdram_tim3			= 0x02f7e45f,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x000b3215,
	.temp_alert_config		= 0x08000a05,
	.emif_ddr_phy_ctlr_1_init	= 0x0E28420d,
	.emif_ddr_phy_ctlr_1		= 0x0E28420d,
	.emif_ddr_ext_phy_ctrl_1	= 0x04020080,
	.emif_ddr_ext_phy_ctrl_2	= 0x28C518A3,
	.emif_ddr_ext_phy_ctrl_3	= 0x518A3146,
	.emif_ddr_ext_phy_ctrl_4	= 0x0014628C,
	.emif_ddr_ext_phy_ctrl_5	= 0x04010040
};

const struct emif_regs emif_regs_266_mhz_2cs = {
	.sdram_config_init		= 0x80800EBA,
	.sdram_config			= 0x808022BA,
	.ref_ctrl			= 0x0000040D,
	.sdram_tim1			= 0x2A86B419,
	.sdram_tim2			= 0x1025094A,
	.sdram_tim3			= 0x026BA22F,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x000b3215,
	.temp_alert_config		= 0x08000a05,
	.emif_ddr_phy_ctlr_1_init	= 0x0E28420d,
	.emif_ddr_phy_ctlr_1		= 0x0E28420d,
	.emif_ddr_ext_phy_ctrl_1	= 0x04020080,
	.emif_ddr_ext_phy_ctrl_2	= 0x0A414829,
	.emif_ddr_ext_phy_ctrl_3	= 0x14829052,
	.emif_ddr_ext_phy_ctrl_4	= 0x000520A4,
	.emif_ddr_ext_phy_ctrl_5	= 0x04010040
};

const struct emif_regs emif_regs_ddr3_532_mhz_1cs = {
	.sdram_config_init		= 0x61851B32,
	.sdram_config			= 0x61851B32,
	.ref_ctrl			= 0x00001035,
	.sdram_tim1			= 0xCCCF36B3,
	.sdram_tim2			= 0x308F7FDA,
	.sdram_tim3			= 0x027F88A8,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x0007190B,
	.temp_alert_config		= 0x00000000,
	.emif_ddr_phy_ctlr_1_init	= 0x0020420A,
	.emif_ddr_phy_ctlr_1		= 0x0024420A,
	.emif_ddr_ext_phy_ctrl_1	= 0x04040100,
	.emif_ddr_ext_phy_ctrl_2	= 0x00000000,
	.emif_ddr_ext_phy_ctrl_3	= 0x00000000,
	.emif_ddr_ext_phy_ctrl_4	= 0x00000000,
	.emif_ddr_ext_phy_ctrl_5	= 0x04010040,
	.emif_rd_wr_lvl_rmp_win		= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl		= 0x80000000,
	.emif_rd_wr_lvl_ctl		= 0x00000000,
	.emif_rd_wr_exec_thresh		= 0x00000305
};

const struct dmm_lisa_map_regs lisa_map_4G_x_2_x_2 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80740300,
	.dmm_lisa_map_3 = 0xFF020100
};

const u32 ext_phy_ctrl_const_base[EMIF_EXT_PHY_CTRL_CONST_REG] = {
	0x01004010,
	0x00001004,
	0x04010040,
	0x01004010,
	0x00001004,
	0x00000000,
	0x00000000,
	0x00000000,
	0x80080080,
	0x00800800,
	0x08102040,
	0x00000001,
	0x540A8150,
	0xA81502a0,
	0x002A0540,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000077
};

const u32 ddr3_ext_phy_ctrl_const_base[EMIF_EXT_PHY_CTRL_CONST_REG] = {
	0x01004010,
	0x00001004,
	0x04010040,
	0x01004010,
	0x00001004,
	0x00000000,
	0x00000000,
	0x00000000,
	0x80080080,
	0x00800800,
	0x08102040,
	0x00000002,
	0x0,
	0x0,
	0x0,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000057
};

static void emif_get_reg_dump_sdp(u32 emif_nr, const struct emif_regs **regs)
{
	if (omap_revision() == OMAP5432_ES1_0)
		*regs = &emif_regs_ddr3_532_mhz_1cs;
	else
		*regs = &emif_regs_532_mhz_2cs;
}
void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
	__attribute__((weak, alias("emif_get_reg_dump_sdp")));

static void emif_get_dmm_regs_sdp(const struct dmm_lisa_map_regs
						**dmm_lisa_regs)
{
	*dmm_lisa_regs = &lisa_map_4G_x_2_x_2;
}

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
	__attribute__((weak, alias("emif_get_dmm_regs_sdp")));

#else

static const struct lpddr2_device_details dev_4G_S4_details = {
	.type		= LPDDR2_TYPE_S4,
	.density	= LPDDR2_DENSITY_4Gb,
	.io_width	= LPDDR2_IO_WIDTH_32,
	.manufacturer	= LPDDR2_MANUFACTURER_SAMSUNG
};

static void emif_get_device_details_sdp(u32 emif_nr,
		struct lpddr2_device_details *cs0_device_details,
		struct lpddr2_device_details *cs1_device_details)
{
	/* EMIF1 & EMIF2 have identical configuration */
	*cs0_device_details = dev_4G_S4_details;
	*cs1_device_details = dev_4G_S4_details;
}

void emif_get_device_details(u32 emif_nr,
		struct lpddr2_device_details *cs0_device_details,
		struct lpddr2_device_details *cs1_device_details)
	__attribute__((weak, alias("emif_get_device_details_sdp")));

#endif /* CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS */

void do_ext_phy_settings(u32 base, const struct emif_regs *regs)
{
	u32 *ext_phy_ctrl_base = 0;
	u32 *emif_ext_phy_ctrl_base = 0;
	u32 i = 0;

	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;

	ext_phy_ctrl_base = (u32 *) &(regs->emif_ddr_ext_phy_ctrl_1);
	emif_ext_phy_ctrl_base = (u32 *) &(emif->emif_ddr_ext_phy_ctrl_1);

	/* Configure external phy control timing registers */
	for (i = 0; i < EMIF_EXT_PHY_CTRL_TIMING_REG; i++) {
		writel(*ext_phy_ctrl_base, emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(*ext_phy_ctrl_base++, emif_ext_phy_ctrl_base++);
	}

	/*
	 * external phy 6-24 registers do not change with
	 * ddr frequency
	 */
	for (i = 0; i < EMIF_EXT_PHY_CTRL_CONST_REG; i++) {
		writel(ext_phy_ctrl_const_base[i],
					emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(ext_phy_ctrl_const_base[i],
					emif_ext_phy_ctrl_base++);
	}
}

#ifndef CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
static const struct lpddr2_ac_timings timings_jedec_532_mhz = {
	.max_freq	= 532000000,
	.RL		= 8,
	.tRPab		= 21,
	.tRCD		= 18,
	.tWR		= 15,
	.tRASmin	= 42,
	.tRRD		= 10,
	.tWTRx2		= 15,
	.tXSR		= 140,
	.tXPx2		= 15,
	.tRFCab		= 130,
	.tRTPx2		= 15,
	.tCKE		= 3,
	.tCKESR		= 15,
	.tZQCS		= 90,
	.tZQCL		= 360,
	.tZQINIT	= 1000,
	.tDQSCKMAXx2	= 11,
	.tRASmax	= 70,
	.tFAW		= 50
};

static const struct lpddr2_min_tck min_tck = {
	.tRL		= 3,
	.tRP_AB		= 3,
	.tRCD		= 3,
	.tWR		= 3,
	.tRAS_MIN	= 3,
	.tRRD		= 2,
	.tWTR		= 2,
	.tXP		= 2,
	.tRTP		= 2,
	.tCKE		= 3,
	.tCKESR		= 3,
	.tFAW		= 8
};

static const struct lpddr2_ac_timings *ac_timings[MAX_NUM_SPEEDBINS] = {
	&timings_jedec_532_mhz
};

static const struct lpddr2_device_timings dev_4G_S4_timings = {
	.ac_timings	= ac_timings,
	.min_tck	= &min_tck,
};

void emif_get_device_timings_sdp(u32 emif_nr,
		const struct lpddr2_device_timings **cs0_device_timings,
		const struct lpddr2_device_timings **cs1_device_timings)
{
	/* Identical devices on EMIF1 & EMIF2 */
	*cs0_device_timings = &dev_4G_S4_timings;
	*cs1_device_timings = &dev_4G_S4_timings;
}

void emif_get_device_timings(u32 emif_nr,
		const struct lpddr2_device_timings **cs0_device_timings,
		const struct lpddr2_device_timings **cs1_device_timings)
	__attribute__((weak, alias("emif_get_device_timings_sdp")));

#endif /* CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS */

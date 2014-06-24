/*
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TEGRA124_MC_H_
#define _TEGRA124_MC_H_

/**
 * Defines the memory controller registers we need/care about
 */
struct mc_ctlr {
	u32 reserved0[4];			/* offset 0x00 - 0x0C */
	u32 mc_smmu_config;			/* offset 0x10 */
	u32 mc_smmu_tlb_config;			/* offset 0x14 */
	u32 mc_smmu_ptc_config;			/* offset 0x18 */
	u32 mc_smmu_ptb_asid;			/* offset 0x1C */
	u32 mc_smmu_ptb_data;			/* offset 0x20 */
	u32 reserved1[3];			/* offset 0x24 - 0x2C */
	u32 mc_smmu_tlb_flush;			/* offset 0x30 */
	u32 mc_smmu_ptc_flush;			/* offset 0x34 */
	u32 reserved2[6];			/* offset 0x38 - 0x4C */
	u32 mc_emem_cfg;			/* offset 0x50 */
	u32 mc_emem_adr_cfg;			/* offset 0x54 */
	u32 mc_emem_adr_cfg_dev0;		/* offset 0x58 */
	u32 mc_emem_adr_cfg_dev1;		/* offset 0x5C */
	u32 reserved3[12];			/* offset 0x60 - 0x8C */
	u32 mc_emem_arb_reserved[28];		/* offset 0x90 - 0xFC */
	u32 reserved4[338];			/* offset 0x100 - 0x644 */
	u32 mc_video_protect_bom;		/* offset 0x648 */
	u32 mc_video_protect_size_mb;		/* offset 0x64c */
	u32 mc_video_protect_reg_ctrl;		/* offset 0x650 */
};

#define TEGRA_MC_VIDEO_PROTECT_REG_WRITE_ACCESS_ENABLED		(0 << 0)
#define TEGRA_MC_VIDEO_PROTECT_REG_WRITE_ACCESS_DISABLED	(1 << 0)

#endif	/* _TEGRA124_MC_H_ */

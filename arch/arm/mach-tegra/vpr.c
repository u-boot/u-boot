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

/* Tegra vpr routines */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/tegra.h>
#include <asm/arch/mc.h>

/* Configures VPR.  Right now, all we do is turn it off. */
void config_vpr(void)
{
	struct mc_ctlr *mc = (struct mc_ctlr *)NV_PA_MC_BASE;

	/* Turn VPR off */
	writel(0, &mc->mc_video_protect_size_mb);
	writel(TEGRA_MC_VIDEO_PROTECT_REG_WRITE_ACCESS_DISABLED,
	       &mc->mc_video_protect_reg_ctrl);
	/* read back to ensure the write went through */
	readl(&mc->mc_video_protect_reg_ctrl);
}

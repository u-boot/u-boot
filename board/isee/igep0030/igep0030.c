/*
 * (C) Copyright 2010
 * ISEE 2007 SL, <www.iseebcn.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "igep0030.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#ifdef CONFIG_SPL_BUILD
/*
 * Routine: omap_rev_string
 * Description: For SPL builds output board rev
 */
void omap_rev_string(void)
{
}

/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(u32 *mcfg, u32 *ctrla, u32 *ctrlb, u32 *rfr_ctrl,
			   u32 *mr)
{
	*mr = MICRON_V_MR_165;
#ifdef CONFIG_BOOT_NAND
	*mcfg = MICRON_V_MCFG_200(256 << 20);
	*ctrla = MICRON_V_ACTIMA_200;
	*ctrlb = MICRON_V_ACTIMB_200;
	*rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
#else
	if (get_cpu_family() == CPU_OMAP34XX) {
		*mcfg = NUMONYX_V_MCFG_165(256 << 20);
		*ctrla = NUMONYX_V_ACTIMA_165;
		*ctrlb = NUMONYX_V_ACTIMB_165;
		*rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;

	} else {
		*mcfg = NUMONYX_V_MCFG_200(256 << 20);
		*ctrla = NUMONYX_V_ACTIMA_200;
		*ctrlb = NUMONYX_V_ACTIMB_200;
		*rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
	}
#endif
}
#endif

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0);
	return 0;
}
#endif

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	twl4030_power_init();

	dieid_num_r();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_DEFAULT();
}

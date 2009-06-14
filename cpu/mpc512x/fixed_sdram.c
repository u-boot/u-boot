/*
 * (C) Copyright 2007-2009 DENX Software Engineering
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
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mpc512x.h>

/*
 * fixed sdram init:
 * The board doesn't use memory modules that have serial presence
 * detect or similar mechanism for discovery of the DRAM settings
 */
long int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2(msize);
	u32 i;

	/* Initialize IO Control */
	out_be32(&im->io_ctrl.io_control_mem, IOCTRL_MUX_DDR);

	/* Initialize DDR Local Window */
	out_be32(&im->sysconf.ddrlaw.bar, CONFIG_SYS_DDR_BASE & 0xFFFFF000);
	out_be32(&im->sysconf.ddrlaw.ar, msize_log2 - 1);
	sync_law(&im->sysconf.ddrlaw.ar);

	/* Enable DDR */
	out_be32(&im->mddrc.ddr_sys_config, CONFIG_SYS_MDDRC_SYS_CFG_EN);

	/* Initialize DDR Priority Manager */
	out_be32(&im->mddrc.prioman_config1, CONFIG_SYS_MDDRCGRP_PM_CFG1);
	out_be32(&im->mddrc.prioman_config2, CONFIG_SYS_MDDRCGRP_PM_CFG2);
	out_be32(&im->mddrc.hiprio_config, CONFIG_SYS_MDDRCGRP_HIPRIO_CFG);
	out_be32(&im->mddrc.lut_table0_main_upper, CONFIG_SYS_MDDRCGRP_LUT0_MU);
	out_be32(&im->mddrc.lut_table0_main_lower, CONFIG_SYS_MDDRCGRP_LUT0_ML);
	out_be32(&im->mddrc.lut_table1_main_upper, CONFIG_SYS_MDDRCGRP_LUT1_MU);
	out_be32(&im->mddrc.lut_table1_main_lower, CONFIG_SYS_MDDRCGRP_LUT1_ML);
	out_be32(&im->mddrc.lut_table2_main_upper, CONFIG_SYS_MDDRCGRP_LUT2_MU);
	out_be32(&im->mddrc.lut_table2_main_lower, CONFIG_SYS_MDDRCGRP_LUT2_ML);
	out_be32(&im->mddrc.lut_table3_main_upper, CONFIG_SYS_MDDRCGRP_LUT3_MU);
	out_be32(&im->mddrc.lut_table3_main_lower, CONFIG_SYS_MDDRCGRP_LUT3_ML);
	out_be32(&im->mddrc.lut_table4_main_upper, CONFIG_SYS_MDDRCGRP_LUT4_MU);
	out_be32(&im->mddrc.lut_table4_main_lower, CONFIG_SYS_MDDRCGRP_LUT4_ML);
	out_be32(&im->mddrc.lut_table0_alternate_upper, CONFIG_SYS_MDDRCGRP_LUT0_AU);
	out_be32(&im->mddrc.lut_table0_alternate_lower, CONFIG_SYS_MDDRCGRP_LUT0_AL);
	out_be32(&im->mddrc.lut_table1_alternate_upper, CONFIG_SYS_MDDRCGRP_LUT1_AU);
	out_be32(&im->mddrc.lut_table1_alternate_lower, CONFIG_SYS_MDDRCGRP_LUT1_AL);
	out_be32(&im->mddrc.lut_table2_alternate_upper, CONFIG_SYS_MDDRCGRP_LUT2_AU);
	out_be32(&im->mddrc.lut_table2_alternate_lower, CONFIG_SYS_MDDRCGRP_LUT2_AL);
	out_be32(&im->mddrc.lut_table3_alternate_upper, CONFIG_SYS_MDDRCGRP_LUT3_AU);
	out_be32(&im->mddrc.lut_table3_alternate_lower, CONFIG_SYS_MDDRCGRP_LUT3_AL);
	out_be32(&im->mddrc.lut_table4_alternate_upper, CONFIG_SYS_MDDRCGRP_LUT4_AU);
	out_be32(&im->mddrc.lut_table4_alternate_lower, CONFIG_SYS_MDDRCGRP_LUT4_AL);

	/* Initialize MDDRC */
	out_be32(&im->mddrc.ddr_sys_config, CONFIG_SYS_MDDRC_SYS_CFG);
	out_be32(&im->mddrc.ddr_time_config0, CONFIG_SYS_MDDRC_TIME_CFG0);
	out_be32(&im->mddrc.ddr_time_config1, CONFIG_SYS_MDDRC_TIME_CFG1);
	out_be32(&im->mddrc.ddr_time_config2, CONFIG_SYS_MDDRC_TIME_CFG2);

	/* Initialize DDR */
	for (i = 0; i < 10; i++)
		out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);

	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_PCHG_ALL);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_RFSH);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_RFSH);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_INIT_DEV_OP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_EM2);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_PCHG_ALL);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_EM2);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_EM3);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_EN_DLL);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_INIT_DEV_OP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_PCHG_ALL);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_RFSH);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_INIT_DEV_OP);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_OCD_DEFAULT);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_PCHG_ALL);
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_MICRON_NOP);

	/* Start MDDRC */
	out_be32(&im->mddrc.ddr_time_config0, CONFIG_SYS_MDDRC_TIME_CFG0_RUN);
	out_be32(&im->mddrc.ddr_sys_config, CONFIG_SYS_MDDRC_SYS_CFG_RUN);

	return msize;
}

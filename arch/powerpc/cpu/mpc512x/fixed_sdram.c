/*
 * (C) Copyright 2007-2009 DENX Software Engineering
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mpc512x.h>

/*
 * MDDRC Config Runtime Settings
 */
ddr512x_config_t default_mddrc_config = {
	.ddr_sys_config   = CONFIG_SYS_MDDRC_SYS_CFG,
	.ddr_time_config0 = CONFIG_SYS_MDDRC_TIME_CFG0,
	.ddr_time_config1 = CONFIG_SYS_MDDRC_TIME_CFG1,
	.ddr_time_config2 = CONFIG_SYS_MDDRC_TIME_CFG2,
};

u32 default_init_seq[] = {
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_RFSH,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_RFSH,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_MICRON_INIT_DEV_OP,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_EM2,
	CONFIG_SYS_DDRCMD_NOP,
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_EM2,
	CONFIG_SYS_DDRCMD_EM3,
	CONFIG_SYS_DDRCMD_EN_DLL,
	CONFIG_SYS_MICRON_INIT_DEV_OP,
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_RFSH,
	CONFIG_SYS_MICRON_INIT_DEV_OP,
	CONFIG_SYS_DDRCMD_OCD_DEFAULT,
	CONFIG_SYS_DDRCMD_PCHG_ALL,
	CONFIG_SYS_DDRCMD_NOP
};

/*
 * fixed sdram init:
 * The board doesn't use memory modules that have serial presence
 * detect or similar mechanism for discovery of the DRAM settings
 */
long int fixed_sdram(ddr512x_config_t *mddrc_config,
			u32 *dram_init_seq, int seq_sz)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_MAX_RAM_SIZE;
	u32 msize_log2 = __ilog2(msize);
	u32 i;

	/* take default settings and init sequence if necessary */
	if (mddrc_config == NULL)
		mddrc_config = &default_mddrc_config;
	if (dram_init_seq == NULL) {
		dram_init_seq = default_init_seq;
		seq_sz = ARRAY_SIZE(default_init_seq);
	}

	/* Initialize IO Control */
	out_be32(&im->io_ctrl.io_control_mem, CONFIG_SYS_IOCTRL_MUX_DDR);

	/* Initialize DDR Local Window */
	out_be32(&im->sysconf.ddrlaw.bar, CONFIG_SYS_DDR_BASE & 0xFFFFF000);
	out_be32(&im->sysconf.ddrlaw.ar, msize_log2 - 1);
	sync_law(&im->sysconf.ddrlaw.ar);

	/* DDR Enable */
	/*
	 * the "enable" combination: DRAM controller out of reset,
	 * clock enabled, command mode -- BUT leave CKE low for now
	 */
	i = MDDRC_SYS_CFG_EN & ~MDDRC_SYS_CFG_CKE_MASK;
	out_be32(&im->mddrc.ddr_sys_config, i);
	/* maintain 200 microseconds of stable power and clock */
	udelay(200);
	/* apply a NOP, it shouldn't harm */
	out_be32(&im->mddrc.ddr_command, CONFIG_SYS_DDRCMD_NOP);
	/* now assert CKE (high) */
	i |= MDDRC_SYS_CFG_CKE_MASK;
	out_be32(&im->mddrc.ddr_sys_config, i);

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

	/*
	 * Initialize MDDRC
	 *  put MDDRC in CMD mode and
	 *  set the max time between refreshes to 0 during init process
	 */
	out_be32(&im->mddrc.ddr_sys_config,
		mddrc_config->ddr_sys_config | MDDRC_SYS_CFG_CMD_MASK);
	out_be32(&im->mddrc.ddr_time_config0,
		mddrc_config->ddr_time_config0 & MDDRC_REFRESH_ZERO_MASK);
	out_be32(&im->mddrc.ddr_time_config1,
		mddrc_config->ddr_time_config1);
	out_be32(&im->mddrc.ddr_time_config2,
		mddrc_config->ddr_time_config2);

	/* Initialize DDR with either default or supplied init sequence */
	for (i = 0; i < seq_sz; i++)
		out_be32(&im->mddrc.ddr_command, dram_init_seq[i]);

	/* Start MDDRC */
	out_be32(&im->mddrc.ddr_time_config0, mddrc_config->ddr_time_config0);
	out_be32(&im->mddrc.ddr_sys_config, mddrc_config->ddr_sys_config);

	/* Allow for the DLL to startup before accessing data */
	udelay(10);

	msize = get_ram_size(CONFIG_SYS_DDR_BASE, CONFIG_SYS_MAX_RAM_SIZE);
	/* Fix DDR Local Window for new size */
	out_be32(&im->sysconf.ddrlaw.ar, __ilog2(msize) - 1);
	sync_law(&im->sysconf.ddrlaw.ar);

	return msize;
}

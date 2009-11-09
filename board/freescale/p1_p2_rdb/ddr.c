/*
 * Copyright 2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;

extern void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
				   unsigned int ctrl_num);

#define DATARATE_400MHZ 400000000
#define DATARATE_533MHZ 533333333
#define DATARATE_667MHZ 666666666
#define DATARATE_800MHZ 800000000

#define CONFIG_SYS_DDR_CS0_BNDS		0x0000003F
#define CONFIG_SYS_DDR_CS0_CONFIG	0x80014202
#define CONFIG_SYS_DDR_CS0_CONFIG_2	0x00000000
#define CONFIG_SYS_DDR_INIT_ADDR	0x00000000
#define CONFIG_SYS_DDR_INIT_EXT_ADDR	0x00000000
#define CONFIG_SYS_DDR_MODE_CONTROL	0x00000000
#define CONFIG_SYS_DDR_ZQ_CONTROL	0x00000000
#define CONFIG_SYS_DDR_WRLVL_CONTROL	0x00000000
#define CONFIG_SYS_DDR_SR_CNTR		0x00000000
#define CONFIG_SYS_DDR_RCW_1		0x00000000
#define CONFIG_SYS_DDR_RCW_2		0x00000000
#define CONFIG_SYS_DDR_CONTROL		0x43000000	/* Type = DDR2*/
#define CONFIG_SYS_DDR_CONTROL_2	0x24401000
#define CONFIG_SYS_DDR_TIMING_4		0x00000000
#define CONFIG_SYS_DDR_TIMING_5		0x00000000

#define CONFIG_SYS_DDR_TIMING_3_400	0x00010000
#define CONFIG_SYS_DDR_TIMING_0_400	0x00260802
#define CONFIG_SYS_DDR_TIMING_1_400	0x39355322
#define CONFIG_SYS_DDR_TIMING_2_400	0x1f9048ca
#define CONFIG_SYS_DDR_CLK_CTRL_400	0x02800000
#define CONFIG_SYS_DDR_MODE_1_400	0x00480432
#define CONFIG_SYS_DDR_MODE_2_400	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_400	0x06180100

#define CONFIG_SYS_DDR_TIMING_3_533	0x00020000
#define CONFIG_SYS_DDR_TIMING_0_533	0x00260802
#define CONFIG_SYS_DDR_TIMING_1_533	0x4c47c432
#define CONFIG_SYS_DDR_TIMING_2_533	0x0f9848ce
#define CONFIG_SYS_DDR_CLK_CTRL_533	0x02800000
#define CONFIG_SYS_DDR_MODE_1_533	0x00040642
#define CONFIG_SYS_DDR_MODE_2_533	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_533	0x08200100

#define CONFIG_SYS_DDR_TIMING_3_667	0x00030000
#define CONFIG_SYS_DDR_TIMING_0_667	0x55770802
#define CONFIG_SYS_DDR_TIMING_1_667	0x5f599543
#define CONFIG_SYS_DDR_TIMING_2_667	0x0fa074d1
#define CONFIG_SYS_DDR_CLK_CTRL_667	0x02800000
#define CONFIG_SYS_DDR_MODE_1_667	0x00040852
#define CONFIG_SYS_DDR_MODE_2_667	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_667	0x0a280100

#define CONFIG_SYS_DDR_TIMING_3_800	0x00040000
#define CONFIG_SYS_DDR_TIMING_0_800	0x55770802
#define CONFIG_SYS_DDR_TIMING_1_800	0x6f6b6543
#define CONFIG_SYS_DDR_TIMING_2_800	0x0fa074d1
#define CONFIG_SYS_DDR_CLK_CTRL_800	0x02800000
#define CONFIG_SYS_DDR_MODE_1_800	0x00040852
#define CONFIG_SYS_DDR_MODE_2_800	0x00000000
#define CONFIG_SYS_DDR_INTERVAL_800	0x0a280100

fsl_ddr_cfg_regs_t ddr_cfg_regs_400 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_400,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_400,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_400,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_400,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_CONTROL,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL_2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_400,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_400,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_400,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_400,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CONTROL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CONTROL,
	.ddr_sr_cntr = CONFIG_SYS_DDR_SR_CNTR,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_533 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_533,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_533,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_533,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_533,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_CONTROL,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL_2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_533,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_533,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_533,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_533,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CONTROL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CONTROL,
	.ddr_sr_cntr = CONFIG_SYS_DDR_SR_CNTR,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_667 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_667,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_667,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_667,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_667,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_CONTROL,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL_2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_667,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_667,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_667,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_667,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CONTROL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CONTROL,
	.ddr_sr_cntr = CONFIG_SYS_DDR_SR_CNTR,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

fsl_ddr_cfg_regs_t ddr_cfg_regs_800 = {
	.cs[0].bnds = CONFIG_SYS_DDR_CS0_BNDS,
	.cs[0].config = CONFIG_SYS_DDR_CS0_CONFIG,
	.cs[0].config_2 = CONFIG_SYS_DDR_CS0_CONFIG_2,
	.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3_800,
	.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0_800,
	.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1_800,
	.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2_800,
	.ddr_sdram_cfg = CONFIG_SYS_DDR_CONTROL,
	.ddr_sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL_2,
	.ddr_sdram_mode = CONFIG_SYS_DDR_MODE_1_800,
	.ddr_sdram_mode_2 = CONFIG_SYS_DDR_MODE_2_800,
	.ddr_sdram_md_cntl = CONFIG_SYS_DDR_MODE_CONTROL,
	.ddr_sdram_interval = CONFIG_SYS_DDR_INTERVAL_800,
	.ddr_data_init = CONFIG_MEM_INIT_VALUE,
	.ddr_sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL_800,
	.ddr_init_addr = CONFIG_SYS_DDR_INIT_ADDR,
	.ddr_init_ext_addr = CONFIG_SYS_DDR_INIT_EXT_ADDR,
	.timing_cfg_4 = CONFIG_SYS_DDR_TIMING_4,
	.timing_cfg_5 = CONFIG_SYS_DDR_TIMING_5,
	.ddr_zq_cntl = CONFIG_SYS_DDR_ZQ_CONTROL,
	.ddr_wrlvl_cntl = CONFIG_SYS_DDR_WRLVL_CONTROL,
	.ddr_sr_cntr = CONFIG_SYS_DDR_SR_CNTR,
	.ddr_sdram_rcw_1 = CONFIG_SYS_DDR_RCW_1,
	.ddr_sdram_rcw_2 = CONFIG_SYS_DDR_RCW_2
};

/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */

phys_size_t fixed_sdram (void)
{
	sys_info_t sysinfo;
	char buf[32];
	fsl_ddr_cfg_regs_t ddr_cfg_regs;
	size_t ddr_size;
	struct cpu_type *cpu;

	get_sys_info(&sysinfo);
	printf("Configuring DDR for %s MT/s data rate\n",
				strmhz(buf, sysinfo.freqDDRBus));

	if(sysinfo.freqDDRBus <= DATARATE_400MHZ)
		memcpy(&ddr_cfg_regs, &ddr_cfg_regs_400, sizeof(ddr_cfg_regs));
	else if(sysinfo.freqDDRBus <= DATARATE_533MHZ)
		memcpy(&ddr_cfg_regs, &ddr_cfg_regs_533, sizeof(ddr_cfg_regs));
	else if(sysinfo.freqDDRBus <= DATARATE_667MHZ)
		memcpy(&ddr_cfg_regs, &ddr_cfg_regs_667, sizeof(ddr_cfg_regs));
	else if(sysinfo.freqDDRBus <= DATARATE_800MHZ)
		memcpy(&ddr_cfg_regs, &ddr_cfg_regs_800, sizeof(ddr_cfg_regs));
	else
		panic("Unsupported DDR data rate %s MT/s data rate\n",
					strmhz(buf, sysinfo.freqDDRBus));

	cpu = gd->cpu;
	/* P1020 and it's derivatives support max 32bit DDR width */
	if(cpu->soc_ver == SVR_P1020 || cpu->soc_ver == SVR_P1020_E ||
		cpu->soc_ver == SVR_P1011 || cpu->soc_ver == SVR_P1011_E) {
		ddr_cfg_regs.ddr_sdram_cfg |= SDRAM_CFG_32_BE;
		ddr_cfg_regs.cs[0].bnds = 0x0000001F;
		ddr_size = (CONFIG_SYS_SDRAM_SIZE * 1024 * 1024 / 2);
	}
	else
		ddr_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 0);

	return ddr_size;
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = 0;

	dram_size = fixed_sdram();
	set_ddr_laws(0, dram_size, LAW_TRGT_IF_DDR_1);

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	puts("DDR: ");
	return dram_size;
}

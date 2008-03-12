/*
 * (C) Copyright 2007 DENX Software Engineering
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
#include <mpc512x.h>
#include <asm/bitops.h>
#include <command.h>
#include <fdt_support.h>

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN |		\
			 CLOCK_SCCR2_I2C_EN)

#define CSAW_START(start)	((start) & 0xFFFF0000)
#define CSAW_STOP(start, size)	(((start) + (size) - 1) >> 16)

long int fixed_sdram(void);

int board_early_init_f (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 lpcaw;

	/*
	 * Initialize Local Window for the CPLD registers access (CS2 selects
	 * the CPLD chip)
	 */
	im->sysconf.lpcs2aw = CSAW_START(CFG_CPLD_BASE) |
			      CSAW_STOP(CFG_CPLD_BASE, CFG_CPLD_SIZE);
	im->lpc.cs_cfg[2] = CFG_CS2_CFG;

	/*
	 * According to MPC5121e RM, configuring local access windows should
	 * be followed by a dummy read of the config register that was
	 * modified last and an isync
	 */
	lpcaw = im->sysconf.lpcs2aw;
	__asm__ __volatile__ ("isync");

	/*
	 * Disable Boot NOR FLASH write protect - CPLD Reg 8 NOR FLASH Control
	 *
	 * Without this the flash identification routine fails, as it needs to issue
	 * write commands in order to establish the device ID.
	 */
	*((volatile u8 *)(CFG_CPLD_BASE + 0x08)) = 0xC1;

	/*
	 * Enable clocks
	 */
	im->clk.sccr[0] = SCCR1_CLOCKS_EN;
	im->clk.sccr[1] = SCCR2_CLOCKS_EN;

	return 0;
}

long int initdram (int board_type)
{
	u32 msize = 0;

	msize = fixed_sdram ();

	return msize;
}

/*
 * fixed sdram init -- the board doesn't use memory modules that have serial presence
 * detect or similar mechanism for discovery of the DRAM settings
 */
long int fixed_sdram (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = CFG_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2 (msize);
	u32 i;

	/* Initialize IO Control */
	im->io_ctrl.regs[MEM_IDX] = IOCTRL_MUX_DDR;

	/* Initialize DDR Local Window */
	im->sysconf.ddrlaw.bar = CFG_DDR_BASE & 0xFFFFF000;
	im->sysconf.ddrlaw.ar = msize_log2 - 1;

	/*
	 * According to MPC5121e RM, configuring local access windows should
	 * be followed by a dummy read of the config register that was
	 * modified last and an isync
	 */
	i = im->sysconf.ddrlaw.ar;
	__asm__ __volatile__ ("isync");

	/* Enable DDR */
	im->mddrc.ddr_sys_config = CFG_MDDRC_SYS_CFG_EN;

	/* Initialize DDR Priority Manager */
	im->mddrc.prioman_config1 = CFG_MDDRCGRP_PM_CFG1;
	im->mddrc.prioman_config2 = CFG_MDDRCGRP_PM_CFG2;
	im->mddrc.hiprio_config = CFG_MDDRCGRP_HIPRIO_CFG;
	im->mddrc.lut_table0_main_upper = CFG_MDDRCGRP_LUT0_MU;
	im->mddrc.lut_table0_main_lower = CFG_MDDRCGRP_LUT0_ML;
	im->mddrc.lut_table1_main_upper = CFG_MDDRCGRP_LUT1_MU;
	im->mddrc.lut_table1_main_lower = CFG_MDDRCGRP_LUT1_ML;
	im->mddrc.lut_table2_main_upper = CFG_MDDRCGRP_LUT2_MU;
	im->mddrc.lut_table2_main_lower = CFG_MDDRCGRP_LUT2_ML;
	im->mddrc.lut_table3_main_upper = CFG_MDDRCGRP_LUT3_MU;
	im->mddrc.lut_table3_main_lower = CFG_MDDRCGRP_LUT3_ML;
	im->mddrc.lut_table4_main_upper = CFG_MDDRCGRP_LUT4_MU;
	im->mddrc.lut_table4_main_lower = CFG_MDDRCGRP_LUT4_ML;
	im->mddrc.lut_table0_alternate_upper = CFG_MDDRCGRP_LUT0_AU;
	im->mddrc.lut_table0_alternate_lower = CFG_MDDRCGRP_LUT0_AL;
	im->mddrc.lut_table1_alternate_upper = CFG_MDDRCGRP_LUT1_AU;
	im->mddrc.lut_table1_alternate_lower = CFG_MDDRCGRP_LUT1_AL;
	im->mddrc.lut_table2_alternate_upper = CFG_MDDRCGRP_LUT2_AU;
	im->mddrc.lut_table2_alternate_lower = CFG_MDDRCGRP_LUT2_AL;
	im->mddrc.lut_table3_alternate_upper = CFG_MDDRCGRP_LUT3_AU;
	im->mddrc.lut_table3_alternate_lower = CFG_MDDRCGRP_LUT3_AL;
	im->mddrc.lut_table4_alternate_upper = CFG_MDDRCGRP_LUT4_AU;
	im->mddrc.lut_table4_alternate_lower = CFG_MDDRCGRP_LUT4_AL;

	/* Initialize MDDRC */
	im->mddrc.ddr_sys_config = CFG_MDDRC_SYS_CFG;
	im->mddrc.ddr_time_config0 = CFG_MDDRC_TIME_CFG0;
	im->mddrc.ddr_time_config1 = CFG_MDDRC_TIME_CFG1;
	im->mddrc.ddr_time_config2 = CFG_MDDRC_TIME_CFG2;

	/* Initialize DDR */
	for (i = 0; i < 10; i++)
		im->mddrc.ddr_command = CFG_MICRON_NOP;

	im->mddrc.ddr_command = CFG_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CFG_MICRON_NOP;
	im->mddrc.ddr_command = CFG_MICRON_RFSH;
	im->mddrc.ddr_command = CFG_MICRON_NOP;
	im->mddrc.ddr_command = CFG_MICRON_RFSH;
	im->mddrc.ddr_command = CFG_MICRON_NOP;
	im->mddrc.ddr_command = CFG_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CFG_MICRON_NOP;
	im->mddrc.ddr_command = CFG_MICRON_EM2;
	im->mddrc.ddr_command = CFG_MICRON_NOP;
	im->mddrc.ddr_command = CFG_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CFG_MICRON_EM2;
	im->mddrc.ddr_command = CFG_MICRON_EM3;
	im->mddrc.ddr_command = CFG_MICRON_EN_DLL;
	im->mddrc.ddr_command = CFG_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CFG_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CFG_MICRON_RFSH;
	im->mddrc.ddr_command = CFG_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CFG_MICRON_OCD_DEFAULT;
	im->mddrc.ddr_command = CFG_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CFG_MICRON_NOP;

	/* Start MDDRC */
	im->mddrc.ddr_time_config0 = CFG_MDDRC_TIME_CFG0_RUN;
	im->mddrc.ddr_sys_config = CFG_MDDRC_SYS_CFG_RUN;

	return msize;
}

int checkboard (void)
{
	ushort brd_rev = *(vu_short *) (CFG_CPLD_BASE + 0x00);
	uchar cpld_rev = *(vu_char *) (CFG_CPLD_BASE + 0x02);
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile unsigned long *reg;
	int i;

	printf ("Board: ADS5121 rev. 0x%04x (CPLD rev. 0x%02x)\n",
		brd_rev, cpld_rev);

	/* change the slew rate on all pata pins to max */
	reg = (unsigned long *) &(im->io_ctrl.regs[PATA_CE1_IDX]);
	for (i = 0; i < 9; i++)
		reg[i] |= 0x00000003;
	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

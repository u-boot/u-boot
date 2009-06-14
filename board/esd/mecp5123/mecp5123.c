/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009 Dave Srl www.dave.eu
 * (C) Copyright 2009 Stefan Roese <sr@denx.de>
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
#include <asm/bitops.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_NFC_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |	\
			 CLOCK_SCCR2_I2C_EN)

#define CSAW_START(start)	((start) & 0xFFFF0000)
#define CSAW_STOP(start, size)	(((start) + (size) - 1) >> 16)

int eeprom_write_enable(unsigned dev_addr, int state)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	if (dev_addr != CONFIG_SYS_I2C_EEPROM_ADDR)
		return -1;

	if (state == 0)
		setbits_be32(&im->gpio.gpdat, 0x00100000);
	else
		clrbits_be32(&im->gpio.gpdat, 0x00100000);

return 0;
}

/*
 * According to MPC5121e RM, configuring local access windows should
 * be followed by a dummy read of the config register that was
 * modified last and an isync.
 */
static inline void sync_law(volatile void *addr)
{
	in_be32(addr);
	__asm__ __volatile__ ("isync");
}

int board_early_init_f(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 spridr;
	int i;

	/*
	 * Initialize Local Window for NOR FLASH access
	 */
	out_be32(&im->sysconf.lpcs0aw,
		 CSAW_START(CONFIG_SYS_FLASH_BASE) |
		 CSAW_STOP(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_SIZE));
	sync_law(&im->sysconf.lpcs0aw);

	/*
	 * Initialize Local Window for boot access
	 */
	out_be32(&im->sysconf.lpbaw,
		 CSAW_START(0xffb00000) | CSAW_STOP(0xffb00000, 0x00010000));
	sync_law(&im->sysconf.lpbaw);

	/*
	 * Initialize Local Window for VPC3 access
	 */
	out_be32(&im->sysconf.lpcs1aw,
		 CSAW_START(CONFIG_SYS_VPC3_BASE) |
		 CSAW_STOP(CONFIG_SYS_VPC3_BASE, CONFIG_SYS_VPC3_SIZE));
	sync_law(&im->sysconf.lpcs1aw);

	/*
	 * Configure Flash Speed
	 */
	out_be32(&im->lpc.cs_cfg[0], CONFIG_SYS_CS0_CFG);

	/*
	 * Configure VPC3 Speed
	 */
	out_be32(&im->lpc.cs_cfg[1], CONFIG_SYS_CS1_CFG);

	spridr = in_be32(&im->sysconf.spridr);
	if (SVR_MJREV(spridr) >= 2)
		out_be32(&im->lpc.altr, CONFIG_SYS_CS_ALETIMING);

	/*
	 * Enable clocks
	 */
	out_be32(&im->clk.sccr[0], SCCR1_CLOCKS_EN);
	out_be32(&im->clk.sccr[1], SCCR2_CLOCKS_EN);
#if defined(CONFIG_IIM) || defined(CONFIG_CMD_FUSE)
	setbits_be32(&im->clk.sccr[1], CLOCK_SCCR2_IIM_EN);
#endif

	/*
	 * Configure MSCAN clocks
	 */
	for (i=0; i<4; ++i) {
		out_be32(&im->clk.msccr[i], 0x00300000);
		out_be32(&im->clk.msccr[i], 0x00310000);
	}

	/*
	 * Configure GPIO's
	 */
	clrbits_be32(&im->gpio.gpodr, 0x000000e0);
	clrbits_be32(&im->gpio.gpdir, 0x00ef0000);
	setbits_be32(&im->gpio.gpdir, 0x001000e0);
	setbits_be32(&im->gpio.gpdat, 0x00100000);

	return 0;
}

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

phys_size_t initdram(int board_type)
{
	return get_ram_size(0, fixed_sdram());
}

int misc_init_r(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 val;

	/*
	 * Optimize access to profibus chip (VPC3) on the local bus
	 */

	/*
	 * Select 1:1 for LPC_DIV
	 */
	val = in_be32(&im->clk.scfr[0]) & ~SCFR1_LPC_DIV_MASK;
	out_be32(&im->clk.scfr[0], val | (0x1 << SCFR1_LPC_DIV_SHIFT));

	/*
	 * Configure LPC Chips Select Deadcycle Control Register
	 * CS0 - device can drive data 2 clock cycle(s) after CS deassertion
	 * CS1 - device can drive data 1 clock cycle(s) after CS deassertion
	 */
	clrbits_be32(&im->lpc.cs_dccr, 0x000000ff);
	setbits_be32(&im->lpc.cs_dccr, (0x00 << 4) | (0x01 << 0));

	/*
	 * Configure LPC Chips Select Holdcycle Control Register
	 * CS0 - data is valid 2 clock cycle(s) after CS deassertion
	 * CS1 - data is valid 1 clock cycle(s) after CS deassertion
	 */
	clrbits_be32(&im->lpc.cs_hccr, 0x000000ff);
	setbits_be32(&im->lpc.cs_hccr, (0x00 << 4) | (0x01 << 0));

	return 0;
}

static iopin_t ioregs_init[] = {
	/* FUNC1=FEC_RX_DV Sets Next 3 to FEC pads */
	{
		offsetof(struct ioctrl512x, io_control_spdif_txclk), 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=FEC_COL Sets Next 15 to FEC pads */
	{
		offsetof(struct ioctrl512x, io_control_psc0_0), 15, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=SELECT LPC_CS1 */
	{
		offsetof(struct ioctrl512x, io_control_lpc_cs1), 1, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC5_2 */
	{
		offsetof(struct ioctrl512x, io_control_psc5_2), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC5_3 */
	{
		offsetof(struct ioctrl512x, io_control_psc5_3), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC7_3 */
	{
		offsetof(struct ioctrl512x, io_control_psc7_3), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC9_0 */
	{
		offsetof(struct ioctrl512x, io_control_psc9_0), 3, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC10_0 */
	{
		offsetof(struct ioctrl512x, io_control_psc10_0), 3, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC10_3 */
	{
		offsetof(struct ioctrl512x, io_control_psc10_3), 1, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=SELECT PSC11_0 */
	{
		offsetof(struct ioctrl512x, io_control_psc11_0), 4, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC0=SELECT IRQ0 */
	{
		offsetof(struct ioctrl512x, io_control_irq0), 4, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	}
};

static iopin_t rev2_silicon_pci_ioregs_init[] = {
	/* FUNC0=PCI Sets next 54 to PCI pads */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad31), 54, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_DS(0)
	}
};

int checkboard(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 spridr;

	puts("Board: MECP_5123\n");

	/*
	 * Initialize function mux & slew rate IO inter alia on IO
	 * Pins
	 */
	iopin_initialize(ioregs_init, ARRAY_SIZE(ioregs_init));

	spridr = in_be32(&im->sysconf.spridr);
	if (SVR_MJREV(spridr) >= 2)
		iopin_initialize(rev2_silicon_pci_ioregs_init, 1);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

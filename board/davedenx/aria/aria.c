/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009 Dave Srl www.dave.eu
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
#include <asm/mpc512x.h>
#include <fdt_support.h>
#ifdef CONFIG_MISC_INIT_R
#include <i2c.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_NFC_EN |				\
			 CLOCK_SCCR1_PATA_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN |		\
			 CLOCK_SCCR2_DIU_EN |		\
			 CLOCK_SCCR2_I2C_EN)

int board_early_init_f(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 spridr;

	/*
	 * Initialize Local Window for the On Board FPGA access
	 */
	out_be32(&im->sysconf.lpcs2aw,
		CSAW_START(CONFIG_SYS_ARIA_FPGA_BASE) |
		CSAW_STOP(CONFIG_SYS_ARIA_FPGA_BASE, CONFIG_SYS_ARIA_FPGA_SIZE)
	);
	out_be32(&im->lpc.cs_cfg[2], CONFIG_SYS_CS2_CFG);
	sync_law(&im->sysconf.lpcs2aw);

	/*
	 * Initialize Local Window for the On Board SRAM access
	 */
	out_be32(&im->sysconf.lpcs6aw,
		CSAW_START(CONFIG_SYS_ARIA_SRAM_BASE) |
		CSAW_STOP(CONFIG_SYS_ARIA_SRAM_BASE, CONFIG_SYS_ARIA_SRAM_SIZE)
	);
	out_be32(&im->lpc.cs_cfg[6], CONFIG_SYS_CS6_CFG);
	sync_law(&im->sysconf.lpcs6aw);

	/*
	 * Configure Flash Speed
	 */
	out_be32(&im->lpc.cs_cfg[0], CONFIG_SYS_CS0_CFG);

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

	return 0;
}

phys_size_t initdram (int board_type)
{
	return fixed_sdram(NULL, NULL, 0);
}

int misc_init_r(void)
{
	u32 tmp;

	/* we use I2C-2 for on-board eeprom */
	i2c_set_bus_num(2);

	tmp = in_be32((u32*)CONFIG_SYS_ARIA_FPGA_BASE);
	printf("FPGA:  %u-%u.%u.%u\n",
		(tmp & 0xFF000000) >> 24,
		(tmp & 0x00FF0000) >> 16,
		(tmp & 0x0000FF00) >>  8,
		 tmp & 0x000000FF
	);

#ifdef CONFIG_FSL_DIU_FB
# if	!(defined(CONFIG_VIDEO) || defined(CONFIG_CFB_CONSOLE))
	mpc5121_diu_init();
# endif
#endif
	return 0;
}

static  iopin_t ioregs_init[] = {
	/*
	 * FEC
	 */

	/* FEC on PSCx_x*/
	{
		offsetof(struct ioctrl512x, io_control_psc0_0), 5, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	{
		offsetof(struct ioctrl512x, io_control_psc1_0), 10, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	{
		offsetof(struct ioctrl512x, io_control_spdif_txclk), 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},

	/*
	 * DIU
	 */
	/* FUNC2=DIU CLK */
	{
		offsetof(struct ioctrl512x, io_control_psc6_0), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU_HSYNC */
	{
		offsetof(struct ioctrl512x, io_control_psc6_1), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC2=DIUVSYNC Sets Next 26 to DIU Pads */
	{
		offsetof(struct ioctrl512x, io_control_psc6_4), 26, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/*
	 * On board SRAM
	 */
	/* FUNC2=/LPC CS6 */
	{
		offsetof(struct ioctrl512x, io_control_j1850_rx), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(1) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
};

int checkboard (void)
{
	puts("Board: ARIA\n");

	/* initialize function mux & slew rate IO inter alia on IO Pins  */

	iopin_initialize(ioregs_init, ARRAY_SIZE(ioregs_init));

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

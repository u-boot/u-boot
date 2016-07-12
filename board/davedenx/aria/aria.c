/*
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 * (C) Copyright 2009 Dave Srl www.dave.eu
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

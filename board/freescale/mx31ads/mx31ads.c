/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	int i;

	/* CS0: Nor Flash */
	/*
	 * CS0L and CS0A values are from the RedBoot sources by Freescale
	 * and are also equal to those used by Sascha Hauer for the Phytec
	 * i.MX31 board. CS0U is just a slightly optimized hardware default:
	 * the only non-zero field "Wait State Control" is set to half the
	 * default value.
	 */
	static const struct mxc_weimcs cs0 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  0, 15, 0,  0,  0),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(1,  0,   0,   0,  0,  1,  5,  0,  0,  0,   1,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(0,   0,  7,  2,  0,  0,  2,  1,  0,  0,  0,  0,   0,   0)
	};

	mxc_setup_weimcs(0, &cs0);

	/* setup pins for UART1 */
	mx31_gpio_mux(MUX_RXD1__UART1_RXD_MUX);
	mx31_gpio_mux(MUX_TXD1__UART1_TXD_MUX);
	mx31_gpio_mux(MUX_RTS1__UART1_RTS_B);
	mx31_gpio_mux(MUX_CTS1__UART1_CTS_B);

	/* SPI2 */
	mx31_gpio_mux(MUX_CSPI2_SS2__CSPI2_SS2_B);
	mx31_gpio_mux(MUX_CSPI2_SCLK__CSPI2_CLK);
	mx31_gpio_mux(MUX_CSPI2_SPI_RDY__CSPI2_DATAREADY_B);
	mx31_gpio_mux(MUX_CSPI2_MOSI__CSPI2_MOSI);
	mx31_gpio_mux(MUX_CSPI2_MISO__CSPI2_MISO);
	mx31_gpio_mux(MUX_CSPI2_SS0__CSPI2_SS0_B);
	mx31_gpio_mux(MUX_CSPI2_SS1__CSPI2_SS1_B);

	/* start SPI2 clock */
	__REG(CCM_CGR2) = __REG(CCM_CGR2) | (3 << 4);

	/* PBC setup */
	/* Enable UART transceivers also reset the Ethernet/external UART */
	readw(CS4_BASE + 4);

	writew(0x8023, CS4_BASE + 4);

	/* RedBoot also has an empty loop with 100000 iterations here -
	 * clock doesn't run yet */
	for (i = 0; i < 100000; i++)
		;

	/* Clear the reset, toggle the LEDs */
	writew(0xDF, CS4_BASE + 6);

	/* clock still doesn't run */
	for (i = 0; i < 100000; i++)
		;

	/* See 1.5.4 in IMX31ADSE_PERI_BUS_CNTRL_CPLD_RM.pdf */
	readb(CS4_BASE + 8);
	readb(CS4_BASE + 7);
	readb(CS4_BASE + 8);
	readb(CS4_BASE + 7);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = 0x80000100;	/* adress of boot parameters */

	return 0;
}

int checkboard(void)
{
	printf("Board: MX31ADS\n");
	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_CS8900
	rc = cs8900_initialize(0, CONFIG_CS8900_BASE);
#endif
	return rc;
}
#endif

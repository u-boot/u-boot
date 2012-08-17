/*
 *
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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
#include <netdev.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <watchdog.h>
#include <pmic.h>
#include <fsl_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_HW_WATCHDOG
void hw_watchdog_reset(void)
{
	mxc_hw_watchdog_reset();
}
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	/* CS5: CPLD incl. network controller */
	static const struct mxc_weimcs cs5 = {
		/*    sp wp bcd bcs psz pme sync dol cnc wsc ew wws edc */
		CSCR_U(0, 0,  0,  0,  0,  0,   0,  0,  3, 24, 0,  4,  3),
		/*   oea oen ebwa ebwn csa ebc dsz csn psr cre wrap csen */
		CSCR_L(2,  2,   2,   5,  2,  0,  5,  2,  0,  0,   0,   1),
		/*  ebra ebrn rwa rwn mum lah lbn lba dww dct wwu age cnc2 fce*/
		CSCR_A(2,   2,  2,  2,  0,  0,  2,  2,  0,  0,  0,  0,   0,  0)
	};

	mxc_setup_weimcs(5, &cs5);

	/* Setup UART1 and SPI2 pins */
	mx31_uart1_hw_init();
	mx31_spi2_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	u32 val;
	struct pmic *p;

	pmic_init();
	p = get_pmic();

	/* Enable RTC battery */
	pmic_reg_read(p, REG_POWER_CTL0, &val);
	pmic_reg_write(p, REG_POWER_CTL0, val | COINCHEN);
	pmic_reg_write(p, REG_INT_STATUS1, RTCRSTI);
#ifdef CONFIG_HW_WATCHDOG
	mxc_hw_watchdog_enable();
#endif
	return 0;
}

int checkboard(void)
{
	printf("Board: MX31PDK\n");
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

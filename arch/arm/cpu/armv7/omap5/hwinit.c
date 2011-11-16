/*
 *
 * Functions for omap5 based boards.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 *	Sricharan	<r.sricharan@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/armv7.h>
#include <asm/arch/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/sizes.h>
#include <asm/utils.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

u32 *const omap5_revision = (u32 *)OMAP5_SRAM_SCRATCH_OMAP5_REV;

static struct gpio_bank gpio_bank_54xx[6] = {
	{ (void *)OMAP54XX_GPIO1_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO2_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO3_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO4_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO5_BASE, METHOD_GPIO_24XX },
	{ (void *)OMAP54XX_GPIO6_BASE, METHOD_GPIO_24XX },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_54xx;

#ifdef CONFIG_SPL_BUILD
/*
 * Some tuning of IOs for optimal power and performance
 */
void do_io_settings(void)
{
}
#endif

void init_omap_revision(void)
{
	/*
	 * For some of the ES2/ES1 boards ID_CODE is not reliable:
	 * Also, ES1 and ES2 have different ARM revisions
	 * So use ARM revision for identification
	 */
	unsigned int rev = cortex_rev();

	switch (rev) {
	case MIDR_CORTEX_A15_R0P0:
		*omap5_revision = OMAP5430_ES1_0;
	default:
		*omap5_revision = OMAP5430_SILICON_ID_INVALID;
	}
}

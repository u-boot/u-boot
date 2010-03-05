/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/errno.h>
#include <asm/io.h>

#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

u32 get_cpu_rev(void)
{
	int reg;
	int system_rev;

	reg = __raw_readl(ROM_SI_REV);
	switch (reg) {
	case 0x02:
		system_rev = 0x51000 | CHIP_REV_1_1;
		break;
	case 0x10:
		if ((__raw_readl(GPIO1_BASE_ADDR + 0x0) & (0x1 << 22)) == 0)
			system_rev = 0x51000 | CHIP_REV_2_5;
		else
			system_rev = 0x51000 | CHIP_REV_2_0;
		break;
	case 0x20:
		system_rev = 0x51000 | CHIP_REV_3_0;
		break;
	return system_rev;
	default:
		system_rev = 0x51000 | CHIP_REV_1_0;
		break;
	}
	return system_rev;
}


#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u32 cpurev;

	cpurev = get_cpu_rev();
	printf("CPU:   Freescale i.MX51 family %d.%dV at %d MHz\n",
		(cpurev & 0xF0) >> 4,
		(cpurev & 0x0F) >> 4,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	return 0;
}
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
#if defined(CONFIG_FEC_MXC)
extern int fecmxc_initialize(bd_t *bis);
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_FSL_ESDHC
	return fsl_esdhc_mmc_init(bis);
#else
	return 0;
#endif
}


void reset_cpu(ulong addr)
{
	__raw_writew(4, WDOG1_BASE_ADDR);
}

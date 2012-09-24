/*
 * Copyright (C) 2012 Linaro Limited
 * Mathieu Poirier <mathieu.poirier@linaro.org>
 *
 * Based on original code from Joakim Axelsson at ST-Ericsson
 * (C) Copyright 2010 ST-Ericsson
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
#include <asm/io.h>
#include <asm/arch/prcmu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>

#include <asm/arch/hardware.h>

#define CPUID_DB8500V1		0x411fc091
#define CPUID_DB8500V2		0x412fc091
#define ASICID_DB8500V11	0x008500A1

#define CACHE_CONTR_BASE	0xA0412000
/* Cache controller register offsets
 * as found in ARM's technical reference manual
 */
#define CACHE_INVAL_BY_WAY	(CACHE_CONTR_BASE + 0x77C)
#define CACHE_LOCKDOWN_BY_D	(CACHE_CONTR_BASE + 0X900)
#define CACHE_LOCKDOWN_BY_I	(CACHE_CONTR_BASE + 0X904)

static unsigned int read_asicid(void);

static inline unsigned int read_cpuid(void)
{
	unsigned int val;

	/* Main ID register (MIDR) */
	asm("mrc        p15, 0, %0, c0, c0, 0"
	   : "=r" (val)
	   :
	   : "cc");

	return val;
}

static int cpu_is_u8500v11(void)
{
	return read_asicid() == ASICID_DB8500V11;
}

static int cpu_is_u8500v2(void)
{
	return read_cpuid() == CPUID_DB8500V2;
}

static unsigned int read_asicid(void)
{
	unsigned int *address;

	if (cpu_is_u8500v2())
		address = (void *) U8500_ASIC_ID_LOC_V2;
	else
		address = (void *) U8500_ASIC_ID_LOC_ED_V1;

	return readl(address);
}

void cpu_cache_initialization(void)
{
	unsigned int value;
	/* invalidate all cache entries */
	writel(0xFFFF, CACHE_INVAL_BY_WAY);

	/* ways are set to '0' when they are totally
	 * cleaned and invalidated
	 */
	do {
		value = readl(CACHE_INVAL_BY_WAY);
	} while (value & 0xFF);

	/* Invalidate register 9 D and I lockdown */
	writel(0xFF, CACHE_LOCKDOWN_BY_D);
	writel(0xFF, CACHE_LOCKDOWN_BY_I);
}

#ifdef CONFIG_ARCH_CPU_INIT
/*
 * SOC specific cpu init
 */
int arch_cpu_init(void)
{
	db8500_prcmu_init();
	db8500_clocks_init();

	return 0;
}
#endif /* CONFIG_ARCH_CPU_INIT */

#ifdef CONFIG_MMC

int u8500_mmc_power_init(void)
{
	int ret;
	int enable, voltage;
	int ab8500_revision;

	if (!cpu_is_u8500v11() && !cpu_is_u8500v2())
		return 0;

	/* Get AB8500 revision */
	ret = ab8500_read(AB8500_MISC, AB8500_REV_REG);
	if (ret < 0)
		goto out;

	ab8500_revision = ret;

	/*
	 * On v1.1 HREF boards (HREF+), Vaux3 needs to be enabled for the SD
	 * card to work.  This is done by enabling the regulators in the AB8500
	 * via PRCMU I2C transactions.
	 *
	 * This code is derived from the handling of AB8500_LDO_VAUX3 in
	 * ab8500_ldo_enable() and ab8500_ldo_disable() in Linux.
	 *
	 * Turn off and delay is required to have it work across soft reboots.
	 */

	/* Turn off (read-modify-write) */
	ret = ab8500_read(AB8500_REGU_CTRL2,
				AB8500_REGU_VRF1VAUX3_REGU_REG);
	if (ret < 0)
		goto out;

	enable = ret;

	/* Turn off */
	ret = ab8500_write(AB8500_REGU_CTRL2,
			AB8500_REGU_VRF1VAUX3_REGU_REG,
			enable & ~LDO_VAUX3_ENABLE_MASK);
	if (ret < 0)
		goto out;

	udelay(10 * 1000);

	/* Set the voltage to 2.91 V or 2.9 V without overriding VRF1 value */
	ret = ab8500_read(AB8500_REGU_CTRL2,
			AB8500_REGU_VRF1VAUX3_SEL_REG);
	if (ret < 0)
		goto out;

	voltage = ret;

	if (ab8500_revision < 0x20) {
		voltage &= ~LDO_VAUX3_SEL_MASK;
		voltage |= LDO_VAUX3_SEL_2V9;
	} else {
		voltage &= ~LDO_VAUX3_V2_SEL_MASK;
		voltage |= LDO_VAUX3_V2_SEL_2V91;
	}

	ret = ab8500_write(AB8500_REGU_CTRL2,
			AB8500_REGU_VRF1VAUX3_SEL_REG, voltage);
	if (ret < 0)
		goto out;

	/* Turn on the supply */
	enable &= ~LDO_VAUX3_ENABLE_MASK;
	enable |= LDO_VAUX3_ENABLE_VAL;

	ret = ab8500_write(AB8500_REGU_CTRL2,
			AB8500_REGU_VRF1VAUX3_REGU_REG, enable);

out:
	return ret;
}
#endif /* CONFIG_MMC */

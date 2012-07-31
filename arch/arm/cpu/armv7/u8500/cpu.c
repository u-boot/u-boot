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
#define ASICID_DB8500V11	0x008500A1

static unsigned int read_asicid(void)
{
	unsigned int *address = (void *)U8500_BOOTROM_BASE
				+ U8500_BOOTROM_ASIC_ID_OFFSET;
	return readl(address);
}

static int cpu_is_u8500v11(void)
{
	return read_asicid() == ASICID_DB8500V11;
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

#define LDO_VAUX3_MASK		0x3
#define LDO_VAUX3_ENABLE	0x1
#define VAUX3_VOLTAGE_2_9V	0xd

#define AB8500_REGU_CTRL2	0x4
#define AB8500_REGU_VRF1VAUX3_REGU_REG	0x040A
#define AB8500_REGU_VRF1VAUX3_SEL_REG	0x0421

int u8500_mmc_power_init(void)
{
	int ret;
	int val;

	if (!cpu_is_u8500v11())
		return 0;

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

	ret = prcmu_i2c_read(AB8500_REGU_CTRL2, AB8500_REGU_VRF1VAUX3_REGU_REG);
	if (ret < 0)
		goto out;

	val = ret;

	/* Turn off */
	ret = prcmu_i2c_write(AB8500_REGU_CTRL2, AB8500_REGU_VRF1VAUX3_REGU_REG,
							val & ~LDO_VAUX3_MASK);
	if (ret < 0)
		goto out;

	udelay(10 * 1000);

	/* Set the voltage to 2.9V */
	ret = prcmu_i2c_write(AB8500_REGU_CTRL2,
				AB8500_REGU_VRF1VAUX3_SEL_REG,
				VAUX3_VOLTAGE_2_9V);
	if (ret < 0)
		goto out;

	val = val & ~LDO_VAUX3_MASK;
	val = val | LDO_VAUX3_ENABLE;

	/* Turn on the supply */
	ret = prcmu_i2c_write(AB8500_REGU_CTRL2,
				AB8500_REGU_VRF1VAUX3_REGU_REG, val);

out:
	return ret;
}
#endif /* CONFIG_MMC */

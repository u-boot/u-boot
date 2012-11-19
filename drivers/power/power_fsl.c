/*
 *  Copyright (C) 2011 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
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
#include <spi.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <errno.h>

#if defined(CONFIG_POWER_SPI)
static u32 pmic_spi_prepare_tx(u32 reg, u32 *val, u32 write)
{
	return (write << 31) | (reg << 25) | (*val & 0x00FFFFFF);
}
#endif

int pmic_init(unsigned char bus)
{
	static const char name[] = "FSL_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->number_of_regs = PMIC_NUM_OF_REGS;

#if defined(CONFIG_POWER_SPI)
	p->interface = PMIC_SPI;
	p->bus = CONFIG_FSL_PMIC_BUS;
	p->hw.spi.cs = CONFIG_FSL_PMIC_CS;
	p->hw.spi.clk = CONFIG_FSL_PMIC_CLK;
	p->hw.spi.mode = CONFIG_FSL_PMIC_MODE;
	p->hw.spi.bitlen = CONFIG_FSL_PMIC_BITLEN;
	p->hw.spi.flags = SPI_XFER_BEGIN | SPI_XFER_END;
	p->hw.spi.prepare_tx = pmic_spi_prepare_tx;
#elif defined(CONFIG_POWER_I2C)
	p->interface = PMIC_I2C;
	p->hw.i2c.addr = CONFIG_SYS_FSL_PMIC_I2C_ADDR;
	p->hw.i2c.tx_num = 3;
	p->bus = bus;
#else
#error "You must select CONFIG_POWER_SPI or CONFIG_PMIC_I2C"
#endif

	return 0;
}

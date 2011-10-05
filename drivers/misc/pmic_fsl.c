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
#include <pmic.h>
#include <fsl_pmic.h>

static u32 pmic_spi_prepare_tx(u32 reg, u32 *val, u32 write)
{
	if ((val == NULL) && (write))
		return *val & ~(1 << 31);
	else
		return (write << 31) | (reg << 25) | (*val & 0x00FFFFFF);
}

int pmic_init(void)
{
	struct pmic *p = get_pmic();
	static const char name[] = "FSL_PMIC";

	puts("Board PMIC init\n");

	p->name = name;
	p->interface = PMIC_SPI;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->bus = CONFIG_FSL_PMIC_BUS;

	p->hw.spi.cs = CONFIG_FSL_PMIC_CS;
	p->hw.spi.clk = CONFIG_FSL_PMIC_CLK;
	p->hw.spi.mode = CONFIG_FSL_PMIC_MODE;
	p->hw.spi.bitlen = CONFIG_FSL_PMIC_BITLEN;
	p->hw.spi.flags = SPI_XFER_BEGIN | SPI_XFER_END;
	p->hw.spi.prepare_tx = pmic_spi_prepare_tx;

	return 0;
}

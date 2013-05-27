/*
 * Common fdt based SPI driver front end
 *
 * Copyright (c) 2013 NVIDIA Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
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
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra20/tegra20_sflash.h>
#include <asm/arch-tegra20/tegra20_slink.h>
#include <asm/arch-tegra114/tegra114_spi.h>
#include <spi.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

struct fdt_spi_driver {
	int compat;
	int max_ctrls;
	int (*init)(int *node_list, int count);
	int (*claim_bus)(struct spi_slave *slave);
	int (*release_bus)(struct spi_slave *slave);
	int (*cs_is_valid)(unsigned int bus, unsigned int cs);
	struct spi_slave *(*setup_slave)(unsigned int bus, unsigned int cs,
					unsigned int max_hz, unsigned int mode);
	void (*free_slave)(struct spi_slave *slave);
	void (*cs_activate)(struct spi_slave *slave);
	void (*cs_deactivate)(struct spi_slave *slave);
	int (*xfer)(struct spi_slave *slave, unsigned int bitlen,
		    const void *data_out, void *data_in, unsigned long flags);
};

static struct fdt_spi_driver fdt_spi_drivers[] = {
#ifdef CONFIG_TEGRA20_SFLASH
	{
		.compat		= COMPAT_NVIDIA_TEGRA20_SFLASH,
		.max_ctrls	= 1,
		.init		= tegra20_spi_init,
		.claim_bus	= tegra20_spi_claim_bus,
		.cs_is_valid	= tegra20_spi_cs_is_valid,
		.setup_slave	= tegra20_spi_setup_slave,
		.free_slave	= tegra20_spi_free_slave,
		.cs_activate	= tegra20_spi_cs_activate,
		.cs_deactivate	= tegra20_spi_cs_deactivate,
		.xfer		= tegra20_spi_xfer,
	},
#endif
#ifdef CONFIG_TEGRA20_SLINK
	{
		.compat		= COMPAT_NVIDIA_TEGRA20_SLINK,
		.max_ctrls	= CONFIG_TEGRA_SLINK_CTRLS,
		.init		= tegra30_spi_init,
		.claim_bus	= tegra30_spi_claim_bus,
		.cs_is_valid	= tegra30_spi_cs_is_valid,
		.setup_slave	= tegra30_spi_setup_slave,
		.free_slave	= tegra30_spi_free_slave,
		.cs_activate	= tegra30_spi_cs_activate,
		.cs_deactivate	= tegra30_spi_cs_deactivate,
		.xfer		= tegra30_spi_xfer,
	},
#endif
#ifdef CONFIG_TEGRA114_SPI
	{
		.compat		= COMPAT_NVIDIA_TEGRA114_SPI,
		.max_ctrls	= CONFIG_TEGRA114_SPI_CTRLS,
		.init		= tegra114_spi_init,
		.claim_bus	= tegra114_spi_claim_bus,
		.cs_is_valid	= tegra114_spi_cs_is_valid,
		.setup_slave	= tegra114_spi_setup_slave,
		.free_slave	= tegra114_spi_free_slave,
		.cs_activate	= tegra114_spi_cs_activate,
		.cs_deactivate	= tegra114_spi_cs_deactivate,
		.xfer		= tegra114_spi_xfer,
	},
#endif
};

static struct fdt_spi_driver *driver;

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	if (!driver)
		return 0;
	else if (!driver->cs_is_valid)
		return 1;
	else
		return driver->cs_is_valid(bus, cs);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	if (!driver || !driver->setup_slave)
		return NULL;

	return driver->setup_slave(bus, cs, max_hz, mode);
}

void spi_free_slave(struct spi_slave *slave)
{
	if (driver && driver->free_slave)
		return driver->free_slave(slave);
}

static int spi_init_driver(struct fdt_spi_driver *driver)
{
	int count;
	int node_list[driver->max_ctrls];

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "spi",
					   driver->compat,
					   node_list,
					   driver->max_ctrls);
	return driver->init(node_list, count);
}

void spi_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fdt_spi_drivers); i++) {
		driver = &fdt_spi_drivers[i];
		if (!spi_init_driver(driver))
			break;
	}
	if (i == ARRAY_SIZE(fdt_spi_drivers))
		driver = NULL;
}

int spi_claim_bus(struct spi_slave *slave)
{
	if (!driver)
		return 1;
	if (!driver->claim_bus)
		return 0;

	return driver->claim_bus(slave);
}

void spi_release_bus(struct spi_slave *slave)
{
	if (driver && driver->release_bus)
		driver->release_bus(slave);
}

void spi_cs_activate(struct spi_slave *slave)
{
	if (driver && driver->cs_activate)
		driver->cs_activate(slave);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	if (driver && driver->cs_deactivate)
		driver->cs_deactivate(slave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *data_out, void *data_in, unsigned long flags)
{
	if (!driver || !driver->xfer)
		return -1;

	return driver->xfer(slave, bitlen, data_out, data_in, flags);
}

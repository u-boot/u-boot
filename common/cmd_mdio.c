/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc
 * Andy Fleming
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

/*
 * MDIO Commands
 */

#include <common.h>
#include <command.h>
#include <miiphy.h>
#include <phy.h>


static char last_op[2];
static uint last_data;
static uint last_addr_lo;
static uint last_addr_hi;
static uint last_devad_lo;
static uint last_devad_hi;
static uint last_reg_lo;
static uint last_reg_hi;

static int extract_range(char *input, int *plo, int *phi)
{
	char *end;
	*plo = simple_strtol(input, &end, 0);
	if (end == input)
		return -1;

	if ((*end == '-') && *(++end))
		*phi = simple_strtol(end, NULL, 0);
	else if (*end == '\0')
		*phi = *plo;
	else
		return -1;

	return 0;
}

int mdio_write_ranges(struct mii_dev *bus, int addrlo,
			int addrhi, int devadlo, int devadhi,
			int reglo, int reghi, unsigned short data)
{
	int addr, devad, reg;
	int err = 0;

	for (addr = addrlo; addr <= addrhi; addr++) {
		for (devad = devadlo; devad <= devadhi; devad++) {
			for (reg = reglo; reg <= reghi; reg++) {
				err = bus->write(bus, addr, devad, reg, data);

				if (err)
					goto err_out;
			}
		}
	}

err_out:
	return err;
}

int mdio_read_ranges(struct mii_dev *bus, int addrlo,
			int addrhi, int devadlo, int devadhi,
			int reglo, int reghi)
{
	int addr, devad, reg;

	printf("Reading from bus %s\n", bus->name);
	for (addr = addrlo; addr <= addrhi; addr++) {
		printf("PHY at address %d:\n", addr);

		for (devad = devadlo; devad <= devadhi; devad++) {
			for (reg = reglo; reg <= reghi; reg++) {
				int val;

				val = bus->read(bus, addr, devad, reg);
				if (val < 0) {
					printf("Error\n");

					return val;
				}

				if (devad >= 0)
					printf("%d.", devad);

				printf("%d - 0x%x\n", reg, val & 0xffff);
			}
		}
	}

	return 0;
}

/* The register will be in the form [a[-b].]x[-y] */
int extract_reg_range(char *input, int *devadlo, int *devadhi,
		int *reglo, int *reghi)
{
	char *regstr;

	/* use strrchr to find the last string after a '.' */
	regstr = strrchr(input, '.');

	/* If it exists, extract the devad(s) */
	if (regstr) {
		char devadstr[32];

		strncpy(devadstr, input, regstr - input);
		devadstr[regstr - input] = '\0';

		if (extract_range(devadstr, devadlo, devadhi))
			return -1;

		regstr++;
	} else {
		/* Otherwise, we have no devad, and we just got regs */
		*devadlo = *devadhi = MDIO_DEVAD_NONE;

		regstr = input;
	}

	return extract_range(regstr, reglo, reghi);
}

int extract_phy_range(char *const argv[], int argc, struct mii_dev **bus,
		int *addrlo, int *addrhi)
{
	struct phy_device *phydev;

	if ((argc < 1) || (argc > 2))
		return -1;

	/* If there are two arguments, it's busname addr */
	if (argc == 2) {
		*bus = miiphy_get_dev_by_name(argv[0]);

		if (!*bus)
			return -1;

		return extract_range(argv[1], addrlo, addrhi);
	}

	/* It must be one argument, here */

	/*
	 * This argument can be one of two things:
	 * 1) Ethernet device name
	 * 2) Just an address (use the previously-used bus)
	 *
	 * We check all buses for a PHY which is connected to an ethernet
	 * device by the given name.  If none are found, we call
	 * extract_range() on the string, and see if it's an address range.
	 */
	phydev = mdio_phydev_for_ethname(argv[0]);

	if (phydev) {
		*addrlo = *addrhi = phydev->addr;
		*bus = phydev->bus;

		return 0;
	}

	/* It's an address or nothing useful */
	return extract_range(argv[0], addrlo, addrhi);
}

/* ---------------------------------------------------------------- */
static int do_mdio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char op[2];
	int addrlo, addrhi, reglo, reghi, devadlo, devadhi;
	unsigned short	data;
	int pos = argc - 1;
	struct mii_dev *bus;

	if (argc < 2)
		return CMD_RET_USAGE;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op[0] = argv[1][0];
	addrlo = last_addr_lo;
	addrhi = last_addr_hi;
	devadlo = last_devad_lo;
	devadhi = last_devad_hi;
	reglo  = last_reg_lo;
	reghi  = last_reg_hi;
	data   = last_data;

	bus = mdio_get_current_dev();

	if (flag & CMD_FLAG_REPEAT)
		op[0] = last_op[0];

	switch (op[0]) {
	case 'w':
		if (pos > 1)
			data = simple_strtoul(argv[pos--], NULL, 16);
	case 'r':
		if (pos > 1)
			if (extract_reg_range(argv[pos--], &devadlo, &devadhi,
					&reglo, &reghi))
				return -1;

	default:
		if (pos > 1)
			if (extract_phy_range(&(argv[2]), pos - 1, &bus,
					&addrlo, &addrhi))
				return -1;

		break;
	}

	if (op[0] == 'l') {
		mdio_list_devices();

		return 0;
	}

	/* Save the chosen bus */
	miiphy_set_current_dev(bus->name);

	switch (op[0]) {
	case 'w':
		mdio_write_ranges(bus, addrlo, addrhi, devadlo, devadhi,
				reglo, reghi, data);
		break;

	case 'r':
		mdio_read_ranges(bus, addrlo, addrhi, devadlo, devadhi,
				reglo, reghi);
		break;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op[0] = op[0];
	last_addr_lo = addrlo;
	last_addr_hi = addrhi;
	last_devad_lo = devadlo;
	last_devad_hi = devadhi;
	last_reg_lo  = reglo;
	last_reg_hi  = reghi;
	last_data    = data;

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	mdio,	6,	1,	do_mdio,
	"MDIO utility commands",
	"list			- List MDIO buses\n"
	"mdio read <phydev> [<devad>.]<reg> - "
		"read PHY's register at <devad>.<reg>\n"
	"mdio write <phydev> [<devad>.]<reg> <data> - "
		"write PHY's register at <devad>.<reg>\n"
	"<phydev> may be:\n"
	"   <busname>  <addr>\n"
	"   <addr>\n"
	"   <eth name>\n"
	"<addr> <devad>, and <reg> may be ranges, e.g. 1-5.4-0x1f.\n"
);

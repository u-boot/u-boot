/*
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2009
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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
 *
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/ppc4xx_config.h>
#include <asm/io.h>

static void print_configs(int cur_config_nr)
{
	int i;

	for (i = 0; i < ppc4xx_config_count; i++) {
		printf("%-16s - %s", ppc4xx_config_val[i].label,
		       ppc4xx_config_val[i].description);
		if (i == cur_config_nr)
			printf(" ***");
		printf("\n");
	}

}

static int do_chip_config(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int ret;
	int cur_config_nr = -1;
	u8 cur_config[CONFIG_4xx_CONFIG_BLOCKSIZE];

	/*
	 * First switch to correct I2C bus. This is I2C bus 0
	 * for all currently available 4xx derivats.
	 */
	I2C_SET_BUS(0);

#ifdef CONFIG_CMD_EEPROM
	ret = eeprom_read(CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR,
			  CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET,
			  cur_config, CONFIG_4xx_CONFIG_BLOCKSIZE);
#else
	ret = i2c_read(CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR,
		       CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET,
		       1, cur_config, CONFIG_4xx_CONFIG_BLOCKSIZE);
#endif
	if (ret) {
		printf("Error reading EEPROM at addr 0x%x\n",
		       CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR);
		return -1;
	}

	/*
	 * Search the current configuration
	 */
	for (i = 0; i < ppc4xx_config_count; i++) {
		if (memcmp(cur_config, ppc4xx_config_val[i].val,
			   CONFIG_4xx_CONFIG_BLOCKSIZE) == 0)
			cur_config_nr = i;
	}

	if (cur_config_nr == -1) {
		printf("Warning: The I2C bootstrap values don't match any"
		       " of the available options!\n");
		printf("I2C bootstrap EEPROM values are (I2C address 0x%02x):\n",
			CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR);
		for (i = 0; i < CONFIG_4xx_CONFIG_BLOCKSIZE; i++) {
			printf("%02x ", cur_config[i]);
		}
		printf("\n");
	}

	if (argc < 2) {
		printf("Available configurations (I2C address 0x%02x):\n",
		       CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR);
		print_configs(cur_config_nr);
		return 0;
	}

	for (i = 0; i < ppc4xx_config_count; i++) {
		/*
		 * Search for configuration name/label
		 */
		if (strcmp(argv[1], ppc4xx_config_val[i].label) == 0) {
			printf("Using configuration:\n%-16s - %s\n",
			       ppc4xx_config_val[i].label,
			       ppc4xx_config_val[i].description);

#ifdef CONFIG_CMD_EEPROM
			ret = eeprom_write(CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR,
					   CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET,
					   ppc4xx_config_val[i].val,
					   CONFIG_4xx_CONFIG_BLOCKSIZE);
#else
			ret = i2c_write(CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR,
					CONFIG_4xx_CONFIG_I2C_EEPROM_OFFSET,
					1, ppc4xx_config_val[i].val,
					CONFIG_4xx_CONFIG_BLOCKSIZE);
#endif
			udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
			if (ret) {
				printf("Error updating EEPROM at addr 0x%x\n",
				       CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR);
				return -1;
			}

			printf("done (dump via 'i2c md %x 0.1 %x')\n",
			       CONFIG_4xx_CONFIG_I2C_EEPROM_ADDR,
			       CONFIG_4xx_CONFIG_BLOCKSIZE);
			printf("Reset the board for the changes to"
			       " take effect\n");
			return 0;
		}
	}

	printf("Configuration %s not found!\n", argv[1]);
	print_configs(cur_config_nr);
	return -1;
}

U_BOOT_CMD(
	chip_config,	2,	0,	do_chip_config,
	"program the I2C bootstrap EEPROM",
	"[config-label]"
);

/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002, 2010
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>

#include "vcma9.h"
#include "../common/common_util.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */

int board_early_init_f(void)
{
	struct s3c24x0_gpio * const gpio = s3c24x0_get_base_gpio();

	/* set up the I/O ports */
	writel(0x007FFFFF, &gpio->gpacon);
	writel(0x002AAAAA, &gpio->gpbcon);
	writel(0x000002BF, &gpio->gpbup);
	writel(0xAAAAAAAA, &gpio->gpccon);
	writel(0x0000FFFF, &gpio->gpcup);
	writel(0xAAAAAAAA, &gpio->gpdcon);
	writel(0x0000FFFF, &gpio->gpdup);
	writel(0xAAAAAAAA, &gpio->gpecon);
	writel(0x000037F7, &gpio->gpeup);
	writel(0x00000000, &gpio->gpfcon);
	writel(0x00000000, &gpio->gpfup);
	writel(0xFFEAFF5A, &gpio->gpgcon);
	writel(0x0000F0DC, &gpio->gpgup);
	writel(0x0028AAAA, &gpio->gphcon);
	writel(0x00000656, &gpio->gphup);

	/* setup correct IRQ modes for NIC (rising edge mode) */
	writel((readl(&gpio->extint2) & ~(7<<8)) | (4<<8),  &gpio->extint2);

	/* select USB port 2 to be host or device (setup as host for now) */
	writel(readl(&gpio->misccr) | 0x08, &gpio->misccr);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

/*
 * Get some Board/PLD Info
 */

static u8 get_pld_reg(enum vcma9_pld_regs reg)
{
	return readb(VCMA9_PLD_BASE + reg);
}

static u8 get_pld_version(void)
{
	return (get_pld_reg(VCMA9_PLD_ID) >> 4) & 0x0F;
}

static u8 get_pld_revision(void)
{
	return get_pld_reg(VCMA9_PLD_ID) & 0x0F;
}

static uchar get_board_pcb(void)
{
	return ((get_pld_reg(VCMA9_PLD_BOARD) >> 4) & 0x03) + 'A';
}

static u8 get_nr_chips(void)
{
	switch ((get_pld_reg(VCMA9_PLD_SDRAM) >> 4) & 0x0F) {
		case 0: return 4;
		case 1: return 1;
		case 2: return 2;
		default: return 0;
	}
}

static ulong get_chip_size(void)
{
	switch (get_pld_reg(VCMA9_PLD_SDRAM) & 0x0F) {
		case 0: return 16 * (1024*1024);
		case 1: return 32 * (1024*1024);
		case 2: return  8 * (1024*1024);
		case 3: return  8 * (1024*1024);
		default: return 0;
	}
}

static const char *get_chip_geom(void)
{
	switch (get_pld_reg(VCMA9_PLD_SDRAM) & 0x0F) {
		case 0: return "4Mx8x4";
		case 1: return "8Mx8x4";
		case 2: return "2Mx8x4";
		case 3: return "4Mx8x2";
		default: return "unknown";
	}
}

static void vcma9_show_info(char *board_name, char *serial)
{
	printf("Board: %s SN: %s  PCB Rev: %c PLD(%d,%d)\n",
		board_name, serial,
		get_board_pcb(), get_pld_version(), get_pld_revision());
	printf("SDRAM: %d chips %s\n", get_nr_chips(), get_chip_geom());
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_chip_size() * get_nr_chips();
	return 0;
}

/*
 * Check Board Identity:
 */

int checkboard(void)
{
	char s[50];
	int i;
	backup_t *b = (backup_t *) s;

	i = getenv_f("serial#", s, 32);
	if ((i < 0) || strncmp (s, "VCMA9", 5)) {
		get_backup_values (b);
		if (strncmp (b->signature, "MPL\0", 4) != 0) {
			puts ("### No HW ID - assuming VCMA9");
		} else {
			b->serial_name[5] = 0;
			vcma9_show_info(b->serial_name, &b->serial_name[6]);
		}
	} else {
		s[5] = 0;
		vcma9_show_info(s, &s[6]);
	}

	return 0;
}

int board_late_init(void)
{
	/*
	 * check if environment is healthy, otherwise restore values
	 * from shadow copy
	 */
	check_env();
	return 0;
}

void vcma9_print_info(void)
{
	char *s = getenv("serial#");

	if (!s) {
		puts ("### No HW ID - assuming VCMA9");
	} else {
		s[5] = 0;
		vcma9_show_info(s, &s[6]);
	}
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_CS8900
	rc = cs8900_initialize(0, CONFIG_CS8900_BASE);
#endif
	return rc;
}
#endif

/*
 * Hardcoded flash setup:
 * Flash 0 is a non-CFI AMD AM29F400BB flash.
 */
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	info->portwidth = FLASH_CFI_16BIT;
	info->chipwidth = FLASH_CFI_BY16;
	info->interface = FLASH_CFI_X16;
	return 1;
}

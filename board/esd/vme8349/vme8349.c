/*
 * vme8349.c -- esd VME8349 board support
 *
 * Copyright (c) 2008-2009 esd gmbh.
 *
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 * Based on board/mpc8349emds/mpc8349emds.c (and previous 834x releases.)
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
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#include <asm/io.h>
#include <asm/mmu.h>
#include <spd.h>
#include <spd_sdram.h>
#include <i2c.h>
#include <netdev.h>

void ddr_enable_ecc(unsigned int dram_size);

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main memory */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_BASE & LAWBAR_BAR;

	msize = spd_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif

	/* Now check memory size (after ECC is initialized) */
	msize = get_ram_size(0, msize);

	/* return total bus SDRAM size(bytes)  -- DDR */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
#ifdef VME_CADDY2
	puts("Board: esd VME-CADDY/2\n");
#else
	puts("Board: esd VME-CPU/8349\n");
#endif

	return 0;
}

#ifdef VME_CADDY2
int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif

int misc_init_r()
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	clrsetbits_be32(&im->lbus.lcrr, LBCR_LDIS, 0);

	return 0;
}

/*
 * Provide SPD values for spd_sdram(). Both boards (VME-CADDY/2
 * and VME-CADDY/2) have different SDRAM configurations.
 */
#ifdef VME_CADDY2
#define SMALL_RAM	0xff
#define LARGE_RAM	0x00
#else
#define SMALL_RAM	0x00
#define LARGE_RAM	0xff
#endif

#define SPD_VAL(a, b)	(((a) & SMALL_RAM) | ((b) & LARGE_RAM))

static spd_eeprom_t default_spd_eeprom = {
	SPD_VAL(0x80, 0x80),	/* 00 use 128 Bytes */
	SPD_VAL(0x07, 0x07),	/* 01 use 128 Bytes */
	SPD_MEMTYPE_DDR2,	/* 02 type is DDR2 */
	SPD_VAL(0x0d, 0x0d),	/* 03 rows: 13 */
	SPD_VAL(0x09, 0x0a),	/* 04 cols:  9 / 10 */
	SPD_VAL(0x00, 0x00),	/* 05 */
	SPD_VAL(0x40, 0x40),	/* 06 */
	SPD_VAL(0x00, 0x00),	/* 07 */
	SPD_VAL(0x05, 0x05),	/* 08 */
	SPD_VAL(0x30, 0x30),	/* 09 */
	SPD_VAL(0x45, 0x45),	/* 10 */
	SPD_VAL(0x02, 0x02),	/* 11 ecc used */
	SPD_VAL(0x82, 0x82),	/* 12 */
	SPD_VAL(0x10, 0x10),	/* 13 */
	SPD_VAL(0x08, 0x08),	/* 14 */
	SPD_VAL(0x00, 0x00),	/* 15 */
	SPD_VAL(0x0c, 0x0c),	/* 16 */
	SPD_VAL(0x04, 0x08),	/* 17 banks: 4 / 8 */
	SPD_VAL(0x38, 0x38),	/* 18 */
	SPD_VAL(0x00, 0x00),	/* 19 */
	SPD_VAL(0x02, 0x02),	/* 20 */
	SPD_VAL(0x00, 0x00),	/* 21 */
	SPD_VAL(0x03, 0x03),	/* 22 */
	SPD_VAL(0x3d, 0x3d),	/* 23 */
	SPD_VAL(0x45, 0x45),	/* 24 */
	SPD_VAL(0x50, 0x50),	/* 25 */
	SPD_VAL(0x45, 0x45),	/* 26 */
	SPD_VAL(0x3c, 0x3c),	/* 27 */
	SPD_VAL(0x28, 0x28),	/* 28 */
	SPD_VAL(0x3c, 0x3c),	/* 29 */
	SPD_VAL(0x2d, 0x2d),	/* 30 */
	SPD_VAL(0x20, 0x80),	/* 31 */
	SPD_VAL(0x20, 0x20),	/* 32 */
	SPD_VAL(0x27, 0x27),	/* 33 */
	SPD_VAL(0x10, 0x10),	/* 34 */
	SPD_VAL(0x17, 0x17),	/* 35 */
	SPD_VAL(0x3c, 0x3c),	/* 36 */
	SPD_VAL(0x1e, 0x1e),	/* 37 */
	SPD_VAL(0x1e, 0x1e),	/* 38 */
	SPD_VAL(0x00, 0x00),	/* 39 */
	SPD_VAL(0x00, 0x06),	/* 40 */
	SPD_VAL(0x37, 0x37),	/* 41 */
	SPD_VAL(0x4b, 0x7f),	/* 42 */
	SPD_VAL(0x80, 0x80),	/* 43 */
	SPD_VAL(0x18, 0x18),	/* 44 */
	SPD_VAL(0x22, 0x22),	/* 45 */
	SPD_VAL(0x00, 0x00),	/* 46 */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	SPD_VAL(0x10, 0x10),	/* 62 */
	SPD_VAL(0x7e, 0x1d),	/* 63 */
	{ 'e', 's', 'd', '-', 'g', 'm', 'b', 'h' },
	SPD_VAL(0x00, 0x00),	/* 72 */
#ifdef VME_CADDY2
	{ "vme-caddy/2 ram   " }
#else
	{ "vme-cpu/2 ram     " }
#endif
};

int vme8349_read_spd(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int old_bus = I2C_GET_BUS();
	unsigned int l, sum;
	int valid = 0;

	I2C_SET_BUS(0);

	if (i2c_read(chip, addr, alen, buffer, len) == 0)
		if (memcmp(&buffer[64], &default_spd_eeprom.mid[0], 8) == 0) {
			sum = 0;
			for (l = 0; l < 63; l++)
				sum = (sum + buffer[l]) & 0xff;
			if (sum == buffer[63])
				valid = 1;
			else
				printf("Invalid checksum in EEPROM %02x %02x\n",
				       sum, buffer[63]);
		}

	if (valid == 0) {
		memcpy(buffer, (void *)&default_spd_eeprom, len);
		sum = 0;
		for (l = 0; l < 63; l++)
			sum = (sum + buffer[l]) & 0xff;
		if (sum != buffer[63])
			printf("Invalid checksum in FLASH %02x %02x\n",
			       sum, buffer[63]);
		buffer[63] = sum;
	}

	I2C_SET_BUS(old_bus);

	return 0;
}

/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
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
#include <command.h>
#include <netdev.h>
#include <asm/mipsregs.h>
#include "vct.h"

#if defined(CONFIG_VCT_PREMIUM)
#define BOARD_NAME	"PremiumD"
#elif defined(CONFIG_VCT_PLATINUM)
#define BOARD_NAME	"PlatinumD"
#elif defined(CONFIG_VCT_PLATINUMAVC)
#define BOARD_NAME	"PlatinumAVC"
#else
#error "vct: No board variant defined!"
#endif

#if defined(CONFIG_VCT_ONENAND)
#define BOARD_NAME_ADD	" OneNAND"
#else
#define BOARD_NAME_ADD	" NOR"
#endif

int board_early_init_f(void)
{
	/*
	 * First initialize the PIN mulitplexing
	 */
	vct_pin_mux_initialize();

	/*
	 * Init the EBI very early so that FLASH can be accessed
	 */
	ebi_initialize();

	return 0;
}

void _machine_restart(void)
{
	reg_write(DCGU_EN_WDT_RESET(DCGU_BASE), DCGU_MAGIC_WDT);
	reg_write(WDT_TORR(WDT_BASE), 0x00);
	reg_write(WDT_CR(WDT_BASE), 0x1D);

	/*
	 * Now wait for the watchdog to trigger the reset
	 */
	udelay(1000000);
}

/*
 * SDRAM is already configured by the bootstrap code, only return the
 * auto-detected size here
 */
phys_size_t initdram(int board_type)
{
	return get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			    CONFIG_SYS_MBYTES_SDRAM << 20);
}

int checkboard(void)
{
	u32 config0 = read_c0_prid();
	char *s = getenv("serial#");

	if ((config0 & 0xff0000) == PRID_COMP_LEGACY
	    && (config0 & 0xff00) == PRID_IMP_LX4280) {
		puts("Board: MDED \n");
		printf("CPU:   LX4280 id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else if ((config0 & 0xff0000) == PRID_COMP_MIPS
		   && (config0 & 0xff00) == PRID_IMP_VGC) {
		u32 jedec_id = *((u32 *) 0xBEBC71A0);
		if ((((jedec_id) >> 12) & 0xFF) == 0x40) {
			puts("Board: VGCA \n");
		} else if ((((jedec_id) >> 12) & 0xFF) == 0x48
			   || (((jedec_id) >> 12) & 0xFF) == 0x49) {
			puts("Board: VGCB \n");
		}
		printf("CPU:   MIPS 4K id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else if (config0 == 0x19378) {
		printf("CPU:   MIPS 24K id: 0x%02x, rev: 0x%02x\n",
		       (config0 >> 8) & 0xFF, config0 & 0xFF);
	} else {
		printf("Unsupported cpu %d, proc_id=0x%x\n", config0 >> 24,
		       config0);
	}

	printf("Board: Micronas VCT " BOARD_NAME BOARD_NAME_ADD);
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

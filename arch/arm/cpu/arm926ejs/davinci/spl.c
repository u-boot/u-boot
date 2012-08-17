/*
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <nand.h>
#include <asm/arch/dm365_lowlevel.h>
#include <ns16550.h>
#include <malloc.h>
#include <spi_flash.h>

#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT

DECLARE_GLOBAL_DATA_PTR;
/* Define global data structure pointer to it*/
static gd_t gdata __attribute__ ((section(".data")));
static bd_t bdata __attribute__ ((section(".data")));

#else

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)(CONFIG_SYS_NS16550_COM1), '\r');

	NS16550_putc((NS16550_t)(CONFIG_SYS_NS16550_COM1), c);
}

#endif /* CONFIG_SPL_LIBCOMMON_SUPPORT */

inline void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}

void board_init_f(ulong dummy)
{
#ifdef CONFIG_SOC_DM365
	dm36x_lowlevel_init(0);
#endif
#ifdef CONFIG_SOC_DA8XX
	arch_cpu_init();
#endif
	relocate_code(CONFIG_SPL_STACK, NULL, CONFIG_SPL_TEXT_BASE);
}

void board_init_r(gd_t *id, ulong dummy)
{
#ifdef CONFIG_SPL_NAND_LOAD
	nand_init();
	puts("Nand boot...\n");
	nand_boot();
#endif
#ifdef CONFIG_SPL_SPI_LOAD
	mem_malloc_init(CONFIG_SYS_TEXT_BASE - CONFIG_SYS_MALLOC_LEN,
			CONFIG_SYS_MALLOC_LEN);

	gd = &gdata;
	gd->bd = &bdata;
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();          /* serial communications setup */
	gd->have_console = 1;

	puts("SPI boot...\n");
	spi_boot();
#endif
}

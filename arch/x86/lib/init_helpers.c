/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <fdtdec.h>
#include <spi.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

/* Get the top of usable RAM */
__weak ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_size;
}

int calculate_relocation_address(void)
{
	const ulong uboot_size = (uintptr_t)&__bss_end -
			(uintptr_t)&__text_start;
	ulong total_size;
	ulong dest_addr;
	ulong fdt_size = 0;

#if defined(CONFIG_OF_SEPARATE) && defined(CONFIG_OF_CONTROL)
	if (gd->fdt_blob)
		fdt_size = ALIGN(fdt_totalsize(gd->fdt_blob) + 0x1000, 32);
#endif
	total_size = ALIGN(uboot_size, 1 << 12) + CONFIG_SYS_MALLOC_LEN +
		CONFIG_SYS_STACK_SIZE + fdt_size;

	dest_addr = board_get_usable_ram_top(total_size);
	/*
	 * NOTE: All destination address are rounded down to 16-byte
	 *       boundary to satisfy various worst-case alignment
	 *       requirements
	 */
	dest_addr &= ~15;

#if defined(CONFIG_OF_SEPARATE) && defined(CONFIG_OF_CONTROL)
	/*
	 * If the device tree is sitting immediate above our image then we
	 * must relocate it. If it is embedded in the data section, then it
	 * will be relocated with other data.
	 */
	if (gd->fdt_blob) {
		dest_addr -= fdt_size;
		gd->new_fdt = (void *)dest_addr;
		dest_addr &= ~15;
	}
#endif
	/* U-Boot is below the FDT */
	dest_addr -= uboot_size;
	dest_addr &= ~((1 << 12) - 1);
	gd->relocaddr = dest_addr;
	gd->reloc_off = dest_addr - (uintptr_t)&__text_start;

	/* Stack is at the bottom, so it can grow down */
	gd->start_addr_sp = dest_addr - CONFIG_SYS_MALLOC_LEN;

	return 0;
}

int init_cache_f_r(void)
{
	/* Initialise the CPU cache(s) */
	return init_cache();
}

bd_t bd_data;

int init_bd_struct_r(void)
{
	gd->bd = &bd_data;
	memset(gd->bd, 0, sizeof(bd_t));

	return 0;
}

int init_func_spi(void)
{
	puts("SPI:   ");
	spi_init();
	puts("ready\n");
	return 0;
}

int find_fdt(void)
{
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = __dtb_dt_begin;
#elif defined CONFIG_OF_SEPARATE
	/* FDT is at end of image */
	gd->fdt_blob = (ulong *)&_end;
#endif
	/* Allow the early environment to override the fdt address */
	gd->fdt_blob = (void *)getenv_ulong("fdtcontroladdr", 16,
						(uintptr_t)gd->fdt_blob);

	return 0;
}

int prepare_fdt(void)
{
	/* For now, put this check after the console is ready */
	if (fdtdec_prepare_fdt()) {
		panic("** CONFIG_OF_CONTROL defined but no FDT - please see "
			"doc/README.fdt-control");
	}

	return 0;
}

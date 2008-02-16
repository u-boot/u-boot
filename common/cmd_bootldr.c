/*
 * U-boot - bootldr.c
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/bootrom.h>

/*
 * the bootldr command loads an address, checks to see if there
 *   is a Boot stream that the on-chip BOOTROM can understand,
 *   and loads it via the BOOTROM Callback. It is possible
 *   to also add booting from SPI, or TWI, but this function does
 *   not currently support that.
 */

int do_bootldr(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	void *addr;
	uint32_t *data;

	/* Get the address */
	if (argc < 2)
		addr = (void *)load_addr;
	else
		addr = (void *)simple_strtoul(argv[1], NULL, 16);

	/* Check if it is a LDR file */
	data = addr;
#if defined(__ADSPBF54x__) || defined(__ADSPBF52x__)
	if ((*data & 0xFF000000) == 0xAD000000 && data[2] == 0x00000000) {
#else
	if (*data == 0xFF800060 || *data == 0xFF800040 || *data == 0xFF800020) {
#endif
		/* We want to boot from FLASH or SDRAM */
		printf("## Booting ldr image at 0x%p ...\n", addr);

		icache_disable();
		dcache_disable();

		__asm__(
			"jump (%1);"
			:
			: "q7" (addr), "a" (_BOOTROM_MEMBOOT));
	} else
		printf("## No ldr image at address 0x%p\n", addr);

	return 0;
}

U_BOOT_CMD(bootldr, 2, 0, do_bootldr,
	"bootldr - boot ldr image from memory\n",
	"[addr]\n"
	"    - boot ldr image stored in memory\n");

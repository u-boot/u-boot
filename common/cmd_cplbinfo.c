/*
 * cmd_cplbinfo.c - dump the instruction/data cplb tables
 *
 * Copyright (c) 2007-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/cplb.h>
#include <asm/mach-common/bits/mpu.h>

/*
 * Translate the PAGE_SIZE bits into a human string
 */
static const char *cplb_page_size(uint32_t data)
{
	static const char page_size_string_table[][4] = { "1K", "4K", "1M", "4M" };
	return page_size_string_table[(data & PAGE_SIZE_MASK) >> PAGE_SIZE_SHIFT];
}

/*
 * show a hardware cplb table
 */
static void show_cplb_table(uint32_t *addr, uint32_t *data)
{
	int i;
	printf("      Address     Data   Size  Valid  Locked\n");
	for (i = 1; i <= 16; ++i) {
		printf(" %2i 0x%p  0x%05X   %s     %c      %c\n",
			i, (void *)*addr, *data,
			cplb_page_size(*data),
			(*data & CPLB_VALID ? 'Y' : 'N'),
			(*data & CPLB_LOCK ? 'Y' : 'N'));
		++addr;
		++data;
	}
}

/*
 * display current instruction and data cplb tables
 */
int do_cplbinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s CPLB table [%08x]:\n", "Instruction", *(uint32_t *)DMEM_CONTROL);
	show_cplb_table((uint32_t *)ICPLB_ADDR0, (uint32_t *)ICPLB_DATA0);

	printf("%s CPLB table [%08x]:\n", "Data", *(uint32_t *)IMEM_CONTROL);
	show_cplb_table((uint32_t *)DCPLB_ADDR0, (uint32_t *)DCPLB_DATA0);

	return 0;
}

U_BOOT_CMD(
	cplbinfo, 1, 0, do_cplbinfo,
	"display current CPLB tables",
	""
);

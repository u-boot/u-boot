/*
 * U-boot - cmd_cache_dump.c
 *
 * Copyright (c) 2007-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/mpu.h>

static int check_limit(const char *type, size_t start_limit, size_t end_limit, size_t start, size_t end)
{
	if (start >= start_limit && start <= end_limit && \
	    end <= end_limit && end >= start_limit && \
	    start <= end)
		return 0;

	printf("%s limit violation: %zu <= (user:%zu) <= (user:%zu) <= %zu\n",
		type, start_limit, start, end, end_limit);
	return 1;
}

int do_icache_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int cache_status = icache_status();

	if (cache_status)
		icache_disable();

	uint32_t cmd_base, tag, cache_upper, cache_lower;

	size_t way, way_start = 0, way_end = 3;
	size_t sbnk, sbnk_start = 0, sbnk_end = 3;
	size_t set, set_start = 0, set_end = 31;
	size_t dw;

	if (argc > 1) {
		way_start = way_end = simple_strtoul(argv[1], NULL, 10);
		if (argc > 2) {
			sbnk_start = sbnk_end = simple_strtoul(argv[2], NULL, 10);
			if (argc > 3)
				set_start = set_end = simple_strtoul(argv[3], NULL, 10);
		}
	}

	if (check_limit("way", 0, 3, way_start, way_end) || \
	    check_limit("subbank", 0, 3, sbnk_start, sbnk_end) || \
	    check_limit("set", 0, 31, set_start, set_end))
		return 1;

	puts("Way:Subbank:Set: [valid-tag lower upper] {invalid-tag lower upper}...\n");

	for (way = way_start; way <= way_end; ++way) {
		for (sbnk = sbnk_start; sbnk <= sbnk_end; ++sbnk) {
			for (set = set_start; set <= set_end; ++set) {
				printf("%zu:%zu:%2zu: ", way, sbnk, set);
				for (dw = 0; dw < 4; ++dw) {
					if (ctrlc())
						return 1;

					cmd_base = \
						(way  << 26) | \
						(sbnk << 16) | \
						(set  <<  5) | \
						(dw   <<  3);

					/* first read the tag */
					bfin_write_ITEST_COMMAND(cmd_base | 0x0);
					SSYNC();
					tag = bfin_read_ITEST_DATA0();
					printf("%c%08x ", (tag & 0x1 ? ' ' : '{'), tag);

					/* grab the data at this loc */
					bfin_write_ITEST_COMMAND(cmd_base | 0x4);
					SSYNC();
					cache_lower = bfin_read_ITEST_DATA0();
					cache_upper = bfin_read_ITEST_DATA1();
					printf("%08x %08x%c ", cache_lower, cache_upper, (tag & 0x1 ? ' ' : '}'));
				}
				puts("\n");
			}
		}
	}

	if (cache_status)
		icache_enable();

	return 0;
}

U_BOOT_CMD(icache_dump, 4, 0, do_icache_dump,
	"icache_dump - dump current instruction cache\n",
	"[way] [subbank] [set]");

int do_dcache_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 way, bank, subbank, set;
	u32 status, addr;
	u32 dmem_ctl = bfin_read_DMEM_CONTROL();

	for (bank = 0; bank < 2; ++bank) {
		if (!(dmem_ctl & (1 << (DMC1_P - bank))))
			continue;

		for (way = 0; way < 2; ++way)
			for (subbank = 0; subbank < 4; ++subbank) {
				printf("%i:%i:%i:\t", bank, way, subbank);
				for (set = 0; set < 64; ++set) {

					if (ctrlc())
						return 1;

					/* retrieve a cache tag */
					bfin_write_DTEST_COMMAND(
						way << 26 |
						bank << 23 |
						subbank << 16 |
						set << 5
					);
					CSYNC();
					status = bfin_read_DTEST_DATA0();

					/* construct the address using the tag */
					addr = (status & 0xFFFFC800) | (subbank << 12) | (set << 5);

					/* show it */
					if (set && !(set % 4))
						puts("\n\t");
					printf("%c%08x%c%08x%c ", (status & 0x1 ? '[' : '{'), status, (status & 0x2 ? 'd' : ' '), addr, (status & 0x1 ? ']' : '}'));
				}
				puts("\n");
			}
	}

	return 0;
}

U_BOOT_CMD(dcache_dump, 4, 0, do_dcache_dump,
	"dcache_dump - dump current data cache\n",
	"[bank] [way] [subbank] [set]");

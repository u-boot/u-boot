// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for bdinfo command
 *
 * Copyright 2023 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <alist.h>
#include <console.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <test/ut.h>
#include <dm.h>
#include <env.h>
#include <lmb.h>
#include <net.h>
#include <serial.h>
#include <video.h>
#include <vsprintf.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <display_options.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new bdinfo test */
#define BDINFO_TEST(_name, _flags)	UNIT_TEST(_name, _flags, bdinfo)

static int test_num_l(struct unit_test_state *uts, const char *name,
		      ulong value)
{
	ut_assert_nextline("%-12s= 0x%0*lx", name, 2 * (int)sizeof(value),
			   value);

	return 0;
}

static int test_num_ll(struct unit_test_state *uts, const char *name,
		       unsigned long long value)
{
	ut_assert_nextline("%-12s= 0x%.*llx", name, 2 * (int)sizeof(ulong),
			   value);

	return 0;
}

static int test_eth(struct unit_test_state *uts)
{
	const int idx = eth_get_dev_index();
	uchar enetaddr[6];
	char name[10];
	int ret;

	if (idx)
		sprintf(name, "eth%iaddr", idx);
	else
		strcpy(name, "ethaddr");

	ret = eth_env_get_enetaddr_by_index("eth", idx, enetaddr);

	ut_assert_nextline("current eth = %s", eth_get_name());
	if (!ret)
		ut_assert_nextline("%-12s= (not set)", name);
	else
		ut_assert_nextline("%-12s= %pM", name, enetaddr);
	ut_assert_nextline("IP addr     = %s", env_get("ipaddr"));

	return 0;
}

static int test_video_info(struct unit_test_state *uts)
{
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		ut_assert_nextline("%-12s= %s %sactive", "Video", dev->name,
				   device_active(dev) ? "" : "in");
		if (device_active(dev)) {
			struct video_priv *upriv = dev_get_uclass_priv(dev);
			struct video_uc_plat *plat = dev_get_uclass_plat(dev);

			ut_assertok(test_num_ll(uts, "FB base",
						(ulong)upriv->fb));
			if (upriv->copy_fb) {
				ut_assertok(test_num_ll(uts, "FB copy",
							(ulong)upriv->copy_fb));
				ut_assertok(test_num_l(uts, " copy size",
						       plat->copy_size));
			}
			ut_assert_nextline("%-12s= %dx%dx%d", "FB size",
					   upriv->xsize, upriv->ysize,
					   1 << upriv->bpix);
		}
	}

	return 0;
}

static int lmb_test_dump_region(struct unit_test_state *uts,
				struct alist *lmb_rgn_lst, char *name)
{
	struct lmb_region *rgn = lmb_rgn_lst->data;
	unsigned long long base, size, end;
	u32 flags;
	int i;

	ut_assert_nextline(" %s.count = %#x", name, lmb_rgn_lst->count);

	for (i = 0; i < lmb_rgn_lst->count; i++) {
		base = rgn[i].base;
		size = rgn[i].size;
		end = base + size - 1;
		flags = rgn[i].flags;

		if (!IS_ENABLED(CONFIG_SANDBOX) && i == 3) {
			ut_assert_nextlinen(" %s[%d]\t[", name, i);
			continue;
		}
		ut_assert_nextlinen(" %s[%d]\t[%#llx-%#llx], %#llx bytes, flags: ",
				    name, i, base, end, size);
	}

	return 0;
}

static int lmb_test_dump_all(struct unit_test_state *uts)
{
	struct lmb *lmb = lmb_get();

	ut_assert_nextline("lmb_dump_all:");
	ut_assertok(lmb_test_dump_region(uts, &lmb->available_mem, "memory"));
	ut_assertok(lmb_test_dump_region(uts, &lmb->used_mem, "reserved"));

	return 0;
}

static int bdinfo_check_mem(struct unit_test_state *uts)
{
	struct bd_info *bd = gd->bd;
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			ut_assertok(test_num_l(uts, "DRAM bank", i));
			ut_assertok(test_num_ll(uts, "-> start",
						bd->bi_dram[i].start));
			ut_assertok(test_num_ll(uts, "-> size",
						bd->bi_dram[i].size));
		}
	}

	return 0;
}

static int bdinfo_test_all(struct unit_test_state *uts)
{
	ut_assertok(test_num_l(uts, "boot_params", 0));

	ut_assertok(bdinfo_check_mem(uts));

	/* CONFIG_SYS_HAS_SRAM testing not supported */
	ut_assertok(test_num_l(uts, "flashstart", 0));
	ut_assertok(test_num_l(uts, "flashsize", 0));
	ut_assertok(test_num_l(uts, "flashoffset", 0));
	ut_assert_nextline("baudrate    = %lu bps",
			   env_get_ulong("baudrate", 10, 1234));
	ut_assertok(test_num_l(uts, "relocaddr", gd->relocaddr));
	ut_assertok(test_num_l(uts, "reloc off", gd->reloc_off));
	ut_assert_nextline("%-12s= %u-bit", "Build", (uint)sizeof(void *) * 8);

	if (IS_ENABLED(CONFIG_CMD_NET))
		ut_assertok(test_eth(uts));

	/*
	 * Make sure environment variable "fdtcontroladdr" address
	 * matches mapped control DT address.
	 */
	ut_assert(map_to_sysmem(gd->fdt_blob) == env_get_hex("fdtcontroladdr", 0x1234));
	ut_assertok(test_num_l(uts, "fdt_blob",
			       (ulong)map_to_sysmem(gd->fdt_blob)));

	if (IS_ENABLED(CONFIG_VIDEO))
		ut_assertok(test_video_info(uts));

	/* The gd->multi_dtb_fit may not be available, hence, #if below. */
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	ut_assertok(test_num_l(uts, "multi_dtb_fit", (ulong)gd->multi_dtb_fit));
#endif

	if (IS_ENABLED(CONFIG_LMB) && gd->fdt_blob) {
		ut_assertok(lmb_test_dump_all(uts));
		if (IS_ENABLED(CONFIG_OF_REAL))
			ut_assert_nextline("devicetree  = %s", fdtdec_get_srcname());
	}

	if (IS_ENABLED(CONFIG_DM_SERIAL)) {
		struct serial_device_info info;

		ut_assertnonnull(gd->cur_serial_dev);
		ut_assertok(serial_getinfo(gd->cur_serial_dev, &info));

		ut_assertok(test_num_l(uts, "serial addr", info.addr));
		ut_assertok(test_num_l(uts, " width", info.reg_width));
		ut_assertok(test_num_l(uts, " shift", info.reg_shift));
		ut_assertok(test_num_l(uts, " offset", info.reg_offset));
		ut_assertok(test_num_l(uts, " clock", info.clock));
	}

	if (IS_ENABLED(CONFIG_CMD_BDINFO_EXTRA)) {
		ut_assert_nextlinen("stack ptr");
		ut_assertok(test_num_ll(uts, "ram_top ptr",
					(unsigned long long)gd->ram_top));
		ut_assertok(test_num_l(uts, "malloc base", gd_malloc_start()));
	}

	if (IS_ENABLED(CONFIG_X86))
		ut_check_skip_to_linen(uts, " high end   =");

	return 0;
}

static int bdinfo_test_full(struct unit_test_state *uts)
{
	/* Test BDINFO full print */
	ut_assertok(run_commandf("bdinfo"));
	ut_assertok(bdinfo_test_all(uts));
	ut_assertok(run_commandf("bdinfo -a"));
	ut_assertok(bdinfo_test_all(uts));
	ut_assert_console_end();

	return 0;
}
BDINFO_TEST(bdinfo_test_full, UTF_CONSOLE);

static int bdinfo_test_help(struct unit_test_state *uts)
{
	/* Test BDINFO unknown option help text print */
	if (!CONFIG_IS_ENABLED(GETOPT)) {
		ut_asserteq(0, run_commandf("bdinfo -h"));
		ut_assertok(bdinfo_test_all(uts));
	} else {
		ut_asserteq(1, run_commandf("bdinfo -h"));
		ut_assert_nextlinen("bdinfo: invalid option -- h");
		ut_assert_nextlinen("bdinfo - print Board Info structure");
		ut_assert_nextline_empty();
		ut_assert_nextlinen("Usage:");
		ut_assert_nextlinen("bdinfo");
	}
	ut_assert_console_end();

	return 0;
}
BDINFO_TEST(bdinfo_test_help, UTF_CONSOLE);

static int bdinfo_test_memory(struct unit_test_state *uts)
{
	/* Test BDINFO memory layout only print */
	ut_assertok(run_commandf("bdinfo -m"));
	if (!CONFIG_IS_ENABLED(GETOPT))
		ut_assertok(bdinfo_test_all(uts));
	else
		ut_assertok(bdinfo_check_mem(uts));
	ut_assert_console_end();

	return 0;
}
BDINFO_TEST(bdinfo_test_memory, UTF_CONSOLE);

static int bdinfo_test_eth(struct unit_test_state *uts)
{
	/* Test BDINFO ethernet settings only print */
	ut_assertok(run_commandf("bdinfo -e"));
	if (!CONFIG_IS_ENABLED(GETOPT))
		ut_assertok(bdinfo_test_all(uts));
	else if (IS_ENABLED(CONFIG_CMD_NET))
		ut_assertok(test_eth(uts));
	ut_assert_console_end();

	return 0;
}
BDINFO_TEST(bdinfo_test_eth, UTF_CONSOLE);

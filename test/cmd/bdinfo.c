// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for bdinfo command
 *
 * Copyright 2023 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <common.h>
#include <console.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <test/suites.h>
#include <test/ut.h>
#include <dm.h>
#include <env.h>
#include <lmb.h>
#include <net.h>
#include <video.h>
#include <vsprintf.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <display_options.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new bdinfo test */
#define BDINFO_TEST(_name, _flags)	UNIT_TEST(_name, _flags, bdinfo_test)

static void bdinfo_test_num_l(struct unit_test_state *uts,
			      const char *name, ulong value)
{
	ut_assert_nextline("%-12s= 0x%0*lx", name, 2 * (int)sizeof(value), value);
}

static void bdinfo_test_num_ll(struct unit_test_state *uts,
			       const char *name, unsigned long long value)
{
	ut_assert_nextline("%-12s= 0x%.*llx", name, 2 * (int)sizeof(ulong), value);
}

static void test_eth(struct unit_test_state *uts)
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
}

static void test_video_info(struct unit_test_state *uts)
{
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		ut_assert_nextline("%-12s= %s %sactive", "Video", dev->name,
				   device_active(dev) ? "" : "in");
		if (device_active(dev)) {
			struct video_priv *upriv = dev_get_uclass_priv(dev);
			struct video_uc_plat *plat = dev_get_uclass_plat(dev);

			bdinfo_test_num_ll(uts, "FB base", (ulong)upriv->fb);
			if (upriv->copy_fb) {
				bdinfo_test_num_ll(uts, "FB copy",
						   (ulong)upriv->copy_fb);
				bdinfo_test_num_l(uts, " copy size",
						  plat->copy_size);
			}
			ut_assert_nextline("%-12s= %dx%dx%d", "FB size",
					   upriv->xsize, upriv->ysize,
					   1 << upriv->bpix);
		}
	}
}

static void lmb_test_dump_region(struct unit_test_state *uts,
				 struct lmb_region *rgn, char *name)
{
	unsigned long long base, size, end;
	enum lmb_flags flags;
	int i;

	ut_assert_nextline(" %s.cnt = 0x%lx / max = 0x%lx", name, rgn->cnt, rgn->max);

	for (i = 0; i < rgn->cnt; i++) {
		base = rgn->region[i].base;
		size = rgn->region[i].size;
		end = base + size - 1;
		flags = rgn->region[i].flags;

		ut_assert_nextline(" %s[%d]\t[0x%llx-0x%llx], 0x%08llx bytes flags: %x",
				   name, i, base, end, size, flags);
	}
}

static void lmb_test_dump_all(struct unit_test_state *uts, struct lmb *lmb)
{
	ut_assert_nextline("lmb_dump_all:");
	lmb_test_dump_region(uts, &lmb->memory, "memory");
	lmb_test_dump_region(uts, &lmb->reserved, "reserved");
}

static int bdinfo_test_move(struct unit_test_state *uts)
{
	struct bd_info *bd = gd->bd;
	int i;

	/* Test moving the working BDINFO to a new location */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("bdinfo"));

	bdinfo_test_num_l(uts, "boot_params", 0);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			bdinfo_test_num_l(uts, "DRAM bank",  i);
			bdinfo_test_num_ll(uts, "-> start", bd->bi_dram[i].start);
			bdinfo_test_num_ll(uts, "-> size", bd->bi_dram[i].size);
		}
	}

	/* CONFIG_SYS_HAS_SRAM testing not supported */
	bdinfo_test_num_l(uts, "flashstart", 0);
	bdinfo_test_num_l(uts, "flashsize", 0);
	bdinfo_test_num_l(uts, "flashoffset", 0);
	ut_assert_nextline("baudrate    = %lu bps",
			   env_get_ulong("baudrate", 10, 1234));
	bdinfo_test_num_l(uts, "relocaddr", gd->relocaddr);
	bdinfo_test_num_l(uts, "reloc off", gd->reloc_off);
	ut_assert_nextline("%-12s= %u-bit", "Build", (uint)sizeof(void *) * 8);

	if (IS_ENABLED(CONFIG_CMD_NET))
		test_eth(uts);

	/*
	 * Make sure environment variable "fdtcontroladdr" address
	 * matches mapped control DT address.
	 */
	ut_assert(map_to_sysmem(gd->fdt_blob) == env_get_hex("fdtcontroladdr", 0x1234));
	bdinfo_test_num_l(uts, "fdt_blob", (ulong)map_to_sysmem(gd->fdt_blob));
	bdinfo_test_num_l(uts, "new_fdt", (ulong)map_to_sysmem(gd->new_fdt));
	bdinfo_test_num_l(uts, "fdt_size", (ulong)gd->fdt_size);

	if (IS_ENABLED(CONFIG_VIDEO))
		test_video_info(uts);

	/* The gd->multi_dtb_fit may not be available, hence, #if below. */
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	bdinfo_test_num_l(uts, "multi_dtb_fit", (ulong)gd->multi_dtb_fit);
#endif

	if (IS_ENABLED(CONFIG_LMB) && gd->fdt_blob) {
		struct lmb lmb;

		lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);
		lmb_test_dump_all(uts, &lmb);
		if (IS_ENABLED(CONFIG_OF_REAL))
			ut_assert_nextline("devicetree  = %s", fdtdec_get_srcname());
	}

	ut_assertok(ut_check_console_end(uts));

	return 0;
}

BDINFO_TEST(bdinfo_test_move, UT_TESTF_CONSOLE_REC);

int do_ut_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(bdinfo_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(bdinfo_test);

	return cmd_ut_category("bdinfo", "bdinfo_test_", tests, n_ents, argc, argv);
}

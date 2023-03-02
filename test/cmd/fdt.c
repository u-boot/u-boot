// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for fdt command
 *
 * Copyright 2022 Google LLCmap_to_sysmem(fdt));
 */

#include <common.h>
#include <console.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new fdt test */
#define FDT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fdt_test)

/**
 * make_test_fdt() - Create an FDT with just a root node
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_test_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/**
 * make_fuller_fdt() - Create an FDT with root node and properties
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_fuller_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	fdt32_t regs[2] = { cpu_to_fdt32(0x1234), cpu_to_fdt32(0x1000) };

	/*
	 * Assemble the following DT for test purposes:
	 *
	 * / {
	 * 	#address-cells = <0x00000001>;
	 * 	#size-cells = <0x00000001>;
	 * 	compatible = "u-boot,fdt-test";
	 * 	model = "U-Boot FDT test";
	 *
	 *	aliases {
	 *		badalias = "/bad/alias";
	 *		subnodealias = "/test-node@1234/subnode";
	 *		testnodealias = "/test-node@1234";
	 *	};
	 *
	 * 	test-node@1234 {
	 * 		#address-cells = <0x00000000>;
	 * 		#size-cells = <0x00000000>;
	 * 		compatible = "u-boot,fdt-test-device1";
	 * 		clock-names = "fixed", "i2c", "spi", "uart2", "uart1";
	 * 		u-boot,empty-property;
	 * 		clock-frequency = <0x00fde800>;
	 * 		regs = <0x00001234 0x00001000>;
	 *
	 * 		subnode {
	 * 			#address-cells = <0x00000000>;
	 * 			#size-cells = <0x00000000>;
	 * 			compatible = "u-boot,fdt-subnode-test-device";
	 * 		};
	 * 	};
	 * };
	 */

	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);

	ut_assertok(fdt_property_u32(fdt, "#address-cells", 1));
	ut_assertok(fdt_property_u32(fdt, "#size-cells", 1));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "model", "U-Boot FDT test"));

	ut_assert(fdt_begin_node(fdt, "aliases") >= 0);
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "badalias", "/bad/alias"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "subnodealias", "/test-node@1234/subnode"));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "testnodealias", "/test-node@1234"));
	ut_assertok(fdt_end_node(fdt));

	ut_assert(fdt_begin_node(fdt, "test-node@1234") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	/* <string> */
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-test-device1"));
	/* <stringlist> */
	ut_assertok(fdt_property(fdt, "clock-names", "fixed\0i2c\0spi\0uart2\0uart1\0", 26));
	/* <empty> */
	ut_assertok(fdt_property(fdt, "u-boot,empty-property", NULL, 0));
	/*
	 * <u32>
	 * This value is deliberate as it used to break cmd/fdt.c
	 * is_printable_string() implementation.
	 */
	ut_assertok(fdt_property_u32(fdt, "clock-frequency", 16640000));
	/* <prop-encoded-array> */
	ut_assertok(fdt_property(fdt, "regs", &regs, sizeof(regs)));
	ut_assert(fdt_begin_node(fdt, "subnode") >= 0);
	ut_assertok(fdt_property_cell(fdt, "#address-cells", 0));
	ut_assertok(fdt_property_cell(fdt, "#size-cells", 0));
	ut_assertok(fdt_property_string(fdt, "compatible", "u-boot,fdt-subnode-test-device"));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_end_node(fdt));

	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/* Test 'fdt addr' getting/setting address */
static int fdt_test_addr(struct unit_test_state *uts)
{
	const void *fdt_blob, *new_fdt;
	char fdt[256];
	ulong addr;
	int ret;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt addr -c", 0));
	ut_assert_nextline("Control fdt: %08lx",
			   (ulong)map_to_sysmem(gd->fdt_blob));
	ut_assertok(ut_check_console_end(uts));

	/* The working fdt is not set, so this should fail */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_asserteq(CMD_RET_FAILURE, run_command("fdt addr", 0));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Set up a working FDT and try again */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(run_command("fdt addr", 0));
	ut_assert_nextline("Working fdt: %08lx", (ulong)map_to_sysmem(fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Set the working FDT */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_assertok(run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(addr, map_to_sysmem(working_fdt));
	ut_assertok(ut_check_console_end(uts));
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");

	/* Set the control FDT */
	fdt_blob = gd->fdt_blob;
	gd->fdt_blob = NULL;
	ret = run_commandf("fdt addr -c %08x", addr);
	new_fdt = gd->fdt_blob;
	gd->fdt_blob = fdt_blob;
	ut_assertok(ret);
	ut_asserteq(addr, map_to_sysmem(new_fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting an invalid FDT */
	fdt[0] = 123;
	ut_asserteq(1, run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Test detecting an invalid FDT */
	fdt[0] = 123;
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(1, run_commandf("fdt addr"));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_addr, UT_TESTF_CONSOLE_REC);

/* Test 'fdt addr' resizing an fdt */
static int fdt_test_addr_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const int newsize = sizeof(fdt) / 2;
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting and resizing the working FDT to a larger size */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt addr %08x %x", addr, newsize));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* Try shrinking it */
	ut_assertok(run_commandf("fdt addr %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assert_nextline("New length %d < existing length %d, ignoring",
			   (int)sizeof(fdt) / 4, newsize);
	ut_assertok(ut_check_console_end(uts));

	/* ...quietly */
	ut_assertok(run_commandf("fdt addr -q %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* We cannot easily provoke errors in fdt_open_into(), so ignore that */

	return 0;
}
FDT_TEST(fdt_test_addr_resize, UT_TESTF_CONSOLE_REC);

/* Test 'fdt get value' reading an fdt */
static int fdt_test_get_value_string(struct unit_test_state *uts,
				     const char *node, const char *prop,
				     const char *idx,  const char *strres,
				     const int intres)
{
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt get value var %s %s %s",
				 node, prop, idx ? : ""));
	if (strres) {
		ut_asserteq_str(strres, env_get("var"));
	} else {
		ut_asserteq(intres, env_get_hex("var", 0x1234));
	}
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value_common(struct unit_test_state *uts,
				     const char *node)
{
	/* Test getting default element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", NULL, "fixed", 0);

	/* Test getting 0th element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "0", "fixed", 0);

	/* Test getting 1st element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "1", "i2c", 0);

	/* Test getting 2nd element of $node node clock-names property */
	fdt_test_get_value_string(uts, node, "clock-names", "2", "spi", 0);

	/*
	 * Test getting default element of $node node regs property.
	 * The result here is highly unusual, the non-index value read from
	 * integer array is a string of concatenated values from the array,
	 * but only if the array is shorter than 40 characters. Anything
	 * longer is an error. This is a special case for handling hashes.
	 */
	fdt_test_get_value_string(uts, node, "regs", NULL, "3412000000100000", 0);

	/* Test getting 0th element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "0", NULL, 0x1234);

	/* Test getting 1st element of $node node regs property */
	fdt_test_get_value_string(uts, node, "regs", "1", NULL, 0x1000);

	/* Test missing 10th element of $node node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s clock-names 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test missing 10th element of $node node regs property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value ften %s regs 10", node));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of $node node nonexistent property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_commandf("fdt get value fnone %s nonexistent", node));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int fdt_test_get_value(struct unit_test_state *uts)
{
	char fdt[4096];
	ulong addr;
	int ret;

	ut_assertok(make_fuller_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	ret = fdt_test_get_value_common(uts, "/test-node@1234");
	if (!ret)
		ret = fdt_test_get_value_common(uts, "testnodealias");
	if (ret)
		return ret;

	/* Test getting default element of /nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value fnode /nonexistent nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of bad alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vbadalias badalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of nonexistent alias */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value vnoalias noalias nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_BADPATH");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get_value, UT_TESTF_CONSOLE_REC);

int do_ut_fdt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(fdt_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(fdt_test);

	return cmd_ut_category("fdt", "fdt_test_", tests, n_ents, argc, argv);
}

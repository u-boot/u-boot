// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for fit command
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#include <console.h>
#include <image.h>
#include <mapmem.h>
#include <linux/libfdt.h>
#include <u-boot/uuid.h>
#include <test/ut.h>

/* Declare a new fit test */
#define FIT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fit)

/**
 * make_fit_with_uuid() - Create a minimal FIT (FDT) with an install-uuid prop
 *
 * Uses the sequential fdt_create() API to build a minimal FDT root node
 * containing an "install-uuid" property of the specified size, filled with
 * zeroes.
 *
 * @uts: Test state
 * @fdt: Buffer to write FDT into
 * @size: Size of the buffer
 * @uuid_prop_len: Length of the install-uuid property to create (use
 *                 FIT_INSTALL_UUID_LEN for the correct size)
 * @addrp: Returns sandbox address of the FDT
 * Return: 0 on success
 */
static int make_fit_with_uuid(struct unit_test_state *uts, void *fdt,
			      int size, int uuid_prop_len, ulong *addrp)
{
	unsigned char zeros[FIT_INSTALL_UUID_LEN] = { 0 };

	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_property(fdt, FIT_INSTALL_UUID_PROP,
				 zeros, uuid_prop_len));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	*addrp = map_to_sysmem(fdt);

	return 0;
}

/**
 * make_fit_no_uuid() - Create a minimal FIT (FDT) without install-uuid
 *
 * @uts: Test state
 * @fdt: Buffer to write FDT into
 * @size: Size of the buffer
 * @addrp: Returns sandbox address of the FDT
 * Return: 0 on success
 */
static int make_fit_no_uuid(struct unit_test_state *uts, void *fdt,
			    int size, ulong *addrp)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_property_string(fdt, "description",
					"test FIT image"));
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	*addrp = map_to_sysmem(fdt);

	return 0;
}

/**
 * fit_test_setuuid_usage() - Test that 'fit setuuid' with wrong argc fails
 *
 * Verifies that calling the command with no arguments or too many arguments
 * returns CMD_RET_USAGE.
 */
static int fit_test_setuuid_usage(struct unit_test_state *uts)
{
	/* No arguments — should print usage (which also outputs help text) */
	ut_asserteq(1, run_command("fit setuuid", 0));

	/* Too many arguments — should also print usage */
	ut_asserteq(1, run_command("fit setuuid 1000 extra", 0));

	return 0;
}
FIT_TEST(fit_test_setuuid_usage, UTF_CONSOLE);

/**
 * fit_test_setuuid_bad_header() - Test 'fit setuuid' with bad FDT header
 *
 * Writes garbage data into memory and verifies the command detects an
 * invalid FIT/FDT header.
 */
static int fit_test_setuuid_bad_header(struct unit_test_state *uts)
{
	u8 *buf;

	buf = map_sysmem(0x1000, 256);
	memset(buf, 0xab, 256);   /* garbage — not a valid FDT */
	unmap_sysmem(buf);

	ut_asserteq(1, run_command("fit setuuid 1000", 0));
	ut_assert_nextline("Bad FIT header at 0x1000");
	ut_assert_console_end();

	return 0;
}
FIT_TEST(fit_test_setuuid_bad_header, UTF_CONSOLE);

/**
 * fit_test_setuuid_no_prop() - Test 'fit setuuid' on FDT missing the property
 *
 * Creates a valid FDT that has no install-uuid property. The command should
 * report that it cannot set the property.
 */
static int fit_test_setuuid_no_prop(struct unit_test_state *uts)
{
	char fdt[512];
	ulong addr;

	ut_assertok(make_fit_no_uuid(uts, fdt, sizeof(fdt), &addr));

	ut_asserteq(1, run_commandf("fit setuuid %lx", addr));
	ut_assert_nextline("Cannot set %s (missing or wrong-size property)",
			   FIT_INSTALL_UUID_PROP);
	ut_assert_console_end();

	return 0;
}
FIT_TEST(fit_test_setuuid_no_prop, UTF_CONSOLE);

/**
 * fit_test_setuuid_wrong_size() - Test 'fit setuuid' with wrong-size property
 *
 * Creates a valid FDT with an install-uuid property that is too small (8
 * bytes instead of 16). fdt_setprop_inplace() should fail because the sizes
 * don't match.
 */
static int fit_test_setuuid_wrong_size(struct unit_test_state *uts)
{
	char fdt[512];
	ulong addr;

	/* Create property with 8 bytes instead of 16 */
	ut_assertok(make_fit_with_uuid(uts, fdt, sizeof(fdt), 8, &addr));

	ut_asserteq(1, run_commandf("fit setuuid %lx", addr));
	ut_assert_nextline("Cannot set %s (missing or wrong-size property)",
			   FIT_INSTALL_UUID_PROP);
	ut_assert_console_end();

	return 0;
}
FIT_TEST(fit_test_setuuid_wrong_size, UTF_CONSOLE);

/**
 * fit_test_setuuid_success() - Test successful 'fit setuuid' operation
 *
 * Creates a valid FDT with a correctly sized install-uuid property (16
 * bytes, initially all zeroes). Runs the command and verifies:
 *   - The command succeeds
 *   - A "Set install-uuid = <uuid>" line is printed
 *   - The UUID written into the FDT is non-zero
 *   - The UUID is a valid version-4 UUID (variant and version bits)
 */
static int fit_test_setuuid_success(struct unit_test_state *uts)
{
	char fdt[512];
	ulong addr;
	const void *prop;
	int prop_len;
	const unsigned char *uuid_bin;
	unsigned char zeros[FIT_INSTALL_UUID_LEN] = { 0 };
	void *fit;

	ut_assertok(make_fit_with_uuid(uts, fdt, sizeof(fdt),
				       FIT_INSTALL_UUID_LEN, &addr));

	ut_assertok(run_commandf("fit setuuid %lx", addr));

	/* Verify console output: "Set install-uuid = <uuid-string>" */
	ut_assert_nextlinen("Set install-uuid = ");
	ut_assert_console_end();

	/* Read back the property from the FDT */
	fit = map_sysmem(addr, 0);
	prop = fdt_getprop(fit, 0, FIT_INSTALL_UUID_PROP, &prop_len);
	unmap_sysmem(fit);

	ut_assertnonnull(prop);
	ut_asserteq(FIT_INSTALL_UUID_LEN, prop_len);

	/* UUID must not be all zeroes (it was randomised) */
	uuid_bin = prop;
	ut_assert(memcmp(uuid_bin, zeros, FIT_INSTALL_UUID_LEN) != 0);

	/* Verify UUID v4 structure:
	 *   - version nibble (byte 6, high nibble) == 0x4
	 *   - variant bits   (byte 8, top 2 bits)  == 0b10
	 */
	ut_asserteq(0x40, uuid_bin[6] & 0xf0);
	ut_asserteq(0x80, uuid_bin[8] & 0xc0);

	return 0;
}
FIT_TEST(fit_test_setuuid_success, UTF_CONSOLE);

/**
 * fit_test_setuuid_idempotent() - Test running 'fit setuuid' twice
 *
 * Stamps a UUID, reads it, stamps again, and verifies the second UUID
 * differs. This confirms gen_rand_uuid() produces distinct values and
 * that the in-place overwrite works correctly.
 */
static int fit_test_setuuid_idempotent(struct unit_test_state *uts)
{
	char fdt[512];
	ulong addr;
	unsigned char uuid_first[FIT_INSTALL_UUID_LEN];
	unsigned char uuid_second[FIT_INSTALL_UUID_LEN];
	const void *prop;
	int prop_len;
	void *fit;

	ut_assertok(make_fit_with_uuid(uts, fdt, sizeof(fdt),
				       FIT_INSTALL_UUID_LEN, &addr));

	/* First stamp */
	ut_assertok(run_commandf("fit setuuid %lx", addr));
	ut_assert_nextlinen("Set install-uuid = ");
	ut_assert_console_end();

	fit = map_sysmem(addr, 0);
	prop = fdt_getprop(fit, 0, FIT_INSTALL_UUID_PROP, &prop_len);
	ut_assertnonnull(prop);
	memcpy(uuid_first, prop, FIT_INSTALL_UUID_LEN);
	unmap_sysmem(fit);

	/* Second stamp */
	ut_assertok(run_commandf("fit setuuid %lx", addr));
	ut_assert_nextlinen("Set install-uuid = ");
	ut_assert_console_end();

	fit = map_sysmem(addr, 0);
	prop = fdt_getprop(fit, 0, FIT_INSTALL_UUID_PROP, &prop_len);
	ut_assertnonnull(prop);
	memcpy(uuid_second, prop, FIT_INSTALL_UUID_LEN);
	unmap_sysmem(fit);

	/* The two UUIDs should differ (probability of collision ≈ 0) */
	ut_assert(memcmp(uuid_first, uuid_second, FIT_INSTALL_UUID_LEN) != 0);

	return 0;
}
FIT_TEST(fit_test_setuuid_idempotent, UTF_CONSOLE);

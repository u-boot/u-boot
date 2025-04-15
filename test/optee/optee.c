// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019, Theobroma Systems Design und Consulting GmbH
 */

#include <command.h>
#include <errno.h>
#include <fdt_support.h>
#include <log.h>
#include <malloc.h>
#include <tee/optee.h>

#include <linux/sizes.h>

#include <test/ut.h>
#include <test/optee.h>

/* 4k ought to be enough for anybody */
#define FDT_COPY_SIZE	(4 * SZ_1K)

extern u32 __dtb_test_optee_base_begin;
extern u32 __dtb_test_optee_optee_begin;
extern u32 __dtb_test_optee_no_optee_begin;

static void *fdt;
static bool expect_success;

static int optee_test_init(struct unit_test_state *uts)
{
	void *fdt_optee = &__dtb_test_optee_optee_begin;
	void *fdt_no_optee = &__dtb_test_optee_no_optee_begin;
	void *fdt_base = &__dtb_test_optee_base_begin;
	int ret = -ENOMEM;

	ut_assertok(fdt_check_header(fdt_base));
	ut_assertok(fdt_check_header(fdt_optee));
	ut_assertok(fdt_check_header(fdt_no_optee));

	fdt = malloc(FDT_COPY_SIZE);
	if (!fdt)
		return ret;

	/*
	 * Resize the FDT to 4k so that we have room to operate on
	 *
	 * (and relocate it since the memory might be mapped
	 * read-only)
	 */
	ut_assertok(fdt_open_into(fdt_base, fdt, FDT_COPY_SIZE));

	return 0;
}
OPTEE_TEST_INIT(optee_test_init, 0);

static int optee_test_uninit(struct unit_test_state *uts)
{
	free(fdt);

	return 0;
}
OPTEE_TEST_UNINIT(optee_test_uninit, 0);

static int optee_fdt_firmware(struct unit_test_state *uts)
{
	const void *prop;
	int offs, len;

	offs = fdt_path_offset(fdt, "/firmware/optee");
	ut_assert(expect_success ? offs >= 0 : offs < 0);

	/* only continue if we have an optee node */
	if (offs < 0)
		return CMD_RET_SUCCESS;

	prop = fdt_getprop(fdt, offs, "compatible", &len);
	ut_assertok(strncmp((const char *)prop, "linaro,optee-tz", len));

	prop = fdt_getprop(fdt, offs, "method", &len);
	ut_assert(strncmp(prop, "hvc", 3) == 0 || strncmp(prop, "smc", 3) == 0);

	return CMD_RET_SUCCESS;
}

static int optee_fdt_protected_memory(struct unit_test_state *uts)
{
	int offs, subnode;
	bool found;

	offs = fdt_path_offset(fdt, "/firmware/optee");
	ut_assert(expect_success ? offs >= 0 : offs < 0);

	/* only continue if we have an optee node */
	if (offs < 0)
		return CMD_RET_SUCCESS;

	/* optee inserts its memory regions as reserved-memory nodes */
	offs = fdt_subnode_offset(fdt, 0, "reserved-memory");
	ut_assert(offs >= 0);

	subnode = fdt_first_subnode(fdt, offs);
	ut_assert(subnode);

	found = 0;
	while (subnode >= 0) {
		const char *name = fdt_get_name(fdt, subnode, NULL);
		struct fdt_resource res;

		ut_assert(name);

		/* only handle optee reservations */
		if (strncmp(name, "optee", 5))
			continue;

		found = true;

		/* check if this subnode has a reg property */
		ut_assertok(fdt_get_resource(fdt, subnode, "reg", 0, &res));
		subnode = fdt_next_subnode(fdt, subnode);
	}

	ut_assert(found);

	return CMD_RET_SUCCESS;
}

/* (1) Try to copy optee nodes from empty dt */
static int optee_fdt_copy_empty(struct unit_test_state *uts)
{
	void *fdt_no_optee = &__dtb_test_optee_no_optee_begin;

	/* This should still run successfully */
	ut_assertok(optee_copy_fdt_nodes(fdt_no_optee, fdt));

	expect_success = false;
	ut_assertok(optee_fdt_firmware(uts));
	ut_assertok(optee_fdt_protected_memory(uts));

	return 0;
}
OPTEE_TEST(optee_fdt_copy_empty, 0);

/* (2) Try to copy optee nodes from prefilled dt */
static int optee_fdt_copy_prefilled(struct unit_test_state *uts)
{
	void *fdt_optee = &__dtb_test_optee_optee_begin;

	ut_assertok(optee_copy_fdt_nodes(fdt_optee, fdt));

	expect_success = true;
	ut_assertok(optee_fdt_firmware(uts));
	ut_assertok(optee_fdt_protected_memory(uts));

	return 0;
}
OPTEE_TEST(optee_fdt_copy_prefilled, 0);

/* (3) Try to copy OP-TEE nodes into a already filled DT */
static int optee_fdt_copy_already_filled(struct unit_test_state *uts)
{
	void *fdt_optee = &__dtb_test_optee_optee_begin;

	ut_assertok(fdt_open_into(fdt_optee, fdt, FDT_COPY_SIZE));
	ut_assertok(optee_copy_fdt_nodes(fdt_optee, fdt));

	expect_success = true;
	ut_assertok(optee_fdt_firmware(uts));
	ut_assertok(optee_fdt_protected_memory(uts));

	return 0;
}
OPTEE_TEST(optee_fdt_copy_already_filled, 0);

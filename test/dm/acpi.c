// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for ACPI table generation
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <tables_csum.h>
#include <version.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <dm/test.h>
#include <test/ut.h>

#define ACPI_TEST_DEV_NAME	"ABCD"
#define BUF_SIZE		4096

static int testacpi_write_tables(const struct udevice *dev,
				 struct acpi_ctx *ctx)
{
	struct acpi_dmar *dmar;
	int ret;

	dmar = (struct acpi_dmar *)ctx->current;
	acpi_create_dmar(dmar, DMAR_INTR_REMAP);
	ctx->current += sizeof(struct acpi_dmar);
	ret = acpi_add_table(ctx, dmar);
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}

static int testacpi_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, ACPI_TEST_DEV_NAME);
}

struct acpi_ops testacpi_ops = {
	.get_name	= testacpi_get_name,
	.write_tables	= testacpi_write_tables,
};

static const struct udevice_id testacpi_ids[] = {
	{ .compatible = "denx,u-boot-acpi-test" },
	{ }
};

U_BOOT_DRIVER(testacpi_drv) = {
	.name	= "testacpi_drv",
	.of_match	= testacpi_ids,
	.id	= UCLASS_TEST_ACPI,
	ACPI_OPS_PTR(&testacpi_ops)
};

UCLASS_DRIVER(testacpi) = {
	.name		= "testacpi",
	.id		= UCLASS_TEST_ACPI,
};

/* Test ACPI get_name() */
static int dm_test_acpi_get_name(struct unit_test_state *uts)
{
	char name[ACPI_NAME_MAX];
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_ACPI, &dev));
	ut_assertok(acpi_get_name(dev, name));
	ut_asserteq_str(ACPI_TEST_DEV_NAME, name);

	return 0;
}
DM_TEST(dm_test_acpi_get_name, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test acpi_get_table_revision() */
static int dm_test_acpi_get_table_revision(struct unit_test_state *uts)
{
	ut_asserteq(1, acpi_get_table_revision(ACPITAB_MCFG));
	ut_asserteq(2, acpi_get_table_revision(ACPITAB_RSDP));
	ut_asserteq(4, acpi_get_table_revision(ACPITAB_TPM2));
	ut_asserteq(-EINVAL, acpi_get_table_revision(ACPITAB_COUNT));

	return 0;
}
DM_TEST(dm_test_acpi_get_table_revision,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test acpi_create_dmar() */
static int dm_test_acpi_create_dmar(struct unit_test_state *uts)
{
	struct acpi_dmar dmar;

	ut_assertok(acpi_create_dmar(&dmar, DMAR_INTR_REMAP));
	ut_asserteq(DMAR_INTR_REMAP, dmar.flags);
	ut_asserteq(32 - 1, dmar.host_address_width);

	return 0;
}
DM_TEST(dm_test_acpi_create_dmar, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test acpi_fill_header() */
static int dm_test_acpi_fill_header(struct unit_test_state *uts)
{
	struct acpi_table_header hdr;

	/* Make sure these 5 fields are not changed */
	hdr.length = 0x11;
	hdr.revision = 0x22;
	hdr.checksum = 0x33;
	hdr.aslc_revision = 0x44;
	acpi_fill_header(&hdr, "ABCD");

	ut_asserteq_mem("ABCD", hdr.signature, sizeof(hdr.signature));
	ut_asserteq(0x11, hdr.length);
	ut_asserteq(0x22, hdr.revision);
	ut_asserteq(0x33, hdr.checksum);
	ut_asserteq_mem(OEM_ID, hdr.oem_id, sizeof(hdr.oem_id));
	ut_asserteq_mem(OEM_TABLE_ID, hdr.oem_table_id,
			sizeof(hdr.oem_table_id));
	ut_asserteq(U_BOOT_BUILD_DATE, hdr.oem_revision);
	ut_asserteq_mem(ASLC_ID, hdr.aslc_id, sizeof(hdr.aslc_id));
	ut_asserteq(0x44, hdr.aslc_revision);

	return 0;
}
DM_TEST(dm_test_acpi_fill_header, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test ACPI write_tables() */
static int dm_test_acpi_write_tables(struct unit_test_state *uts)
{
	struct acpi_dmar *dmar;
	struct acpi_ctx ctx;
	void *buf;

	buf = malloc(BUF_SIZE);
	ut_assertnonnull(buf);

	acpi_setup_base_tables(&ctx, buf);
	dmar = ctx.current;
	ut_assertok(acpi_write_dev_tables(&ctx));

	/*
	 * We should have two dmar tables, one for each "denx,u-boot-acpi-test"
	 * device
	 */
	ut_asserteq_ptr(dmar + 2, ctx.current);
	ut_asserteq(DMAR_INTR_REMAP, dmar->flags);
	ut_asserteq(32 - 1, dmar->host_address_width);

	ut_asserteq(DMAR_INTR_REMAP, dmar[1].flags);
	ut_asserteq(32 - 1, dmar[1].host_address_width);

	/* Check that the pointers were added correctly */
	ut_asserteq(map_to_sysmem(dmar), ctx.rsdt->entry[0]);
	ut_asserteq(map_to_sysmem(dmar + 1), ctx.rsdt->entry[1]);
	ut_asserteq(0, ctx.rsdt->entry[2]);

	return 0;
}
DM_TEST(dm_test_acpi_write_tables, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test basic ACPI functions */
static int dm_test_acpi_basic(struct unit_test_state *uts)
{
	struct acpi_ctx ctx;

	/* Check align works */
	ctx.current = (void *)5;
	acpi_align(&ctx);
	ut_asserteq_ptr((void *)16, ctx.current);

	/* Check that align does nothing if already aligned */
	acpi_align(&ctx);
	ut_asserteq_ptr((void *)16, ctx.current);
	acpi_align64(&ctx);
	ut_asserteq_ptr((void *)64, ctx.current);
	acpi_align64(&ctx);
	ut_asserteq_ptr((void *)64, ctx.current);

	/* Check incrementing */
	acpi_inc(&ctx, 3);
	ut_asserteq_ptr((void *)67, ctx.current);
	acpi_inc_align(&ctx, 3);
	ut_asserteq_ptr((void *)80, ctx.current);

	return 0;
}
DM_TEST(dm_test_acpi_basic, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test acpi_setup_base_tables */
static int dm_test_acpi_setup_base_tables(struct unit_test_state *uts)
{
	struct acpi_rsdp *rsdp;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;
	struct acpi_ctx ctx;
	void *buf, *end;

	/*
	 * Use an unaligned address deliberately, by allocating an aligned
	 * address and then adding 4 to it
	 */
	buf = memalign(64, BUF_SIZE);
	ut_assertnonnull(buf);
	acpi_setup_base_tables(&ctx, buf + 4);

	rsdp = buf + 16;
	ut_asserteq_ptr(rsdp, ctx.rsdp);
	ut_assertok(memcmp(RSDP_SIG, rsdp->signature, sizeof(rsdp->signature)));
	ut_asserteq(sizeof(*rsdp), rsdp->length);
	ut_assertok(table_compute_checksum(rsdp, 20));
	ut_assertok(table_compute_checksum(rsdp, sizeof(*rsdp)));

	rsdt = PTR_ALIGN((void *)rsdp + sizeof(*rsdp), 16);
	ut_asserteq_ptr(rsdt, ctx.rsdt);
	ut_assertok(memcmp("RSDT", rsdt->header.signature, ACPI_NAME_LEN));
	ut_asserteq(sizeof(*rsdt), rsdt->header.length);
	ut_assertok(table_compute_checksum(rsdt, sizeof(*rsdt)));

	xsdt = PTR_ALIGN((void *)rsdt + sizeof(*rsdt), 16);
	ut_assertok(memcmp("XSDT", xsdt->header.signature, ACPI_NAME_LEN));
	ut_asserteq(sizeof(*xsdt), xsdt->header.length);
	ut_assertok(table_compute_checksum(xsdt, sizeof(*xsdt)));

	end = PTR_ALIGN((void *)xsdt + sizeof(*xsdt), 64);
	ut_asserteq_ptr(end, ctx.current);

	ut_asserteq(map_to_sysmem(rsdt), rsdp->rsdt_address);
	ut_asserteq(map_to_sysmem(xsdt), rsdp->xsdt_address);

	return 0;
}
DM_TEST(dm_test_acpi_setup_base_tables,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

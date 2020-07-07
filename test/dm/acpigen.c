// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for ACPI code generation
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <malloc.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <asm/unaligned.h>
#include <dm/acpi.h>
#include <dm/test.h>
#include <test/ut.h>

/* Maximum size of the ACPI context needed for most tests */
#define ACPI_CONTEXT_SIZE	150

#define TEST_STRING	"frogmore"
#define TEST_STREAM2	"\xfa\xde"

static int alloc_context(struct acpi_ctx **ctxp)
{
	struct acpi_ctx *ctx;

	*ctxp = NULL;
	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;
	ctx->base = malloc(ACPI_CONTEXT_SIZE);
	if (!ctx->base) {
		free(ctx);
		return -ENOMEM;
	}
	ctx->current = ctx->base;
	*ctxp = ctx;

	return 0;
}

static void free_context(struct acpi_ctx **ctxp)
{
	free((*ctxp)->base);
	free(*ctxp);
	*ctxp = NULL;
}

/* Test emitting simple types and acpigen_get_current() */
static int dm_test_acpi_emit_simple(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_byte(ctx, 0x23);
	ut_asserteq(1, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x23, *(u8 *)ptr);

	acpigen_emit_word(ctx, 0x1234);
	ut_asserteq(3, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x1234, get_unaligned((u16 *)(ptr + 1)));

	acpigen_emit_dword(ctx, 0x87654321);
	ut_asserteq(7, acpigen_get_current(ctx) - ptr);
	ut_asserteq(0x87654321, get_unaligned((u32 *)(ptr + 3)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_simple, 0);

/* Test emitting a stream */
static int dm_test_acpi_emit_stream(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_stream(ctx, TEST_STREAM2, 2);
	ut_asserteq(2, acpigen_get_current(ctx) - ptr);
	ut_asserteq((u8)TEST_STREAM2[0], ptr[0]);
	ut_asserteq((u8)TEST_STREAM2[1], ptr[1]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_stream, 0);

/* Test emitting a string */
static int dm_test_acpi_emit_string(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_emit_string(ctx, TEST_STRING);
	ut_asserteq(sizeof(TEST_STRING), acpigen_get_current(ctx) - ptr);
	ut_asserteq_str(TEST_STRING, (char *)ptr);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_emit_string, 0);

/* Test emitting an interrupt descriptor */
static int dm_test_acpi_interrupt(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	struct irq irq;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_assertok(irq_get_by_index(dev, 0, &irq));

	/* See a-test, property interrupts-extended in the device tree */
	ut_asserteq(3, acpi_device_write_interrupt_irq(ctx, &irq));
	ut_asserteq(9, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_INTERRUPT, ptr[0]);
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(0x19, ptr[3]);
	ut_asserteq(1, ptr[4]);
	ut_asserteq(3, get_unaligned((u32 *)(ptr + 5)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_interrupt, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

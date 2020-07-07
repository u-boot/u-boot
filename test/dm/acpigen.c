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
#include <asm/gpio.h>
#include <asm/unaligned.h>
#include <dm/acpi.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>
#include "acpi.h"

/* Maximum size of the ACPI context needed for most tests */
#define ACPI_CONTEXT_SIZE	150

#define TEST_STRING	"frogmore"
#define TEST_STRING2	"ranch"
#define TEST_STREAM2	"\xfa\xde"

#define TEST_INT8	0x7d
#define TEST_INT16	0x2345
#define TEST_INT32	0x12345678
#define TEST_INT64	0x4567890123456

int acpi_test_alloc_context_size(struct acpi_ctx **ctxp, int size)
{
	struct acpi_ctx *ctx;

	*ctxp = NULL;
	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;
	ctx->base = malloc(size);
	if (!ctx->base) {
		free(ctx);
		return -ENOMEM;
	}
	ctx->ltop = 0;
	ctx->current = ctx->base;
	*ctxp = ctx;

	return 0;
}

int acpi_test_get_length(u8 *ptr)
{
	if (!(*ptr & 0x80))
		return -EINVAL;

	return (*ptr & 0xf) | ptr[1] << 4 | ptr[2] << 12;
}

static int alloc_context(struct acpi_ctx **ctxp)
{
	return acpi_test_alloc_context_size(ctxp, ACPI_CONTEXT_SIZE);
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

/* Test emitting a GPIO descriptor */
static int dm_test_acpi_gpio(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));

	/* This should write GPIO pin 4 (see device tree test.dts ) */
	ut_asserteq(4, acpi_device_write_gpio_desc(ctx, &desc));
	ut_asserteq(35, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);
	ut_asserteq(32, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_GPIO_REVISION_ID, ptr[3]);
	ut_asserteq(ACPI_GPIO_TYPE_IO, ptr[4]);
	ut_asserteq(1, get_unaligned((u16 *)(ptr + 5)));
	ut_asserteq(9, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_GPIO_PULL_UP, ptr[9]);
	ut_asserteq(1234, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 12)));
	ut_asserteq(23, get_unaligned((u16 *)(ptr + 14)));
	ut_asserteq(0, ptr[16]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 17)));
	ut_asserteq(35, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 21)));

	/* pin0 */
	ut_asserteq(4, get_unaligned((u16 *)(ptr + 23)));

	ut_asserteq_str("\\_SB.PINC", (char *)ptr + 25);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_gpio, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test emitting a GPIO descriptor with an interrupt */
static int dm_test_acpi_gpio_irq(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 2, &desc, 0));

	/* This should write GPIO pin 6 (see device tree test.dts ) */
	ut_asserteq(6, acpi_device_write_gpio_desc(ctx, &desc));
	ut_asserteq(35, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);
	ut_asserteq(32, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_GPIO_REVISION_ID, ptr[3]);
	ut_asserteq(ACPI_GPIO_TYPE_INTERRUPT, ptr[4]);
	ut_asserteq(1, get_unaligned((u16 *)(ptr + 5)));
	ut_asserteq(29, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_GPIO_PULL_DOWN, ptr[9]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(4321, get_unaligned((u16 *)(ptr + 12)));
	ut_asserteq(23, get_unaligned((u16 *)(ptr + 14)));
	ut_asserteq(0, ptr[16]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 17)));
	ut_asserteq(35, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 21)));

	/* pin0 */
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 23)));

	ut_asserteq_str("\\_SB.PINC", (char *)ptr + 25);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_gpio_irq, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test emitting either a GPIO or interrupt descriptor */
static int dm_test_acpi_interrupt_or_gpio(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/* This should produce an interrupt, even though it also has a GPIO */
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_asserteq(3, acpi_device_write_interrupt_or_gpio(ctx, dev,
							   "test2-gpios"));
	ut_asserteq(ACPI_DESCRIPTOR_INTERRUPT, ptr[0]);

	/* This has no interrupt so should produce a GPIO */
	ptr = ctx->current;
	ut_assertok(uclass_find_first_device(UCLASS_PANEL_BACKLIGHT, &dev));
	ut_asserteq(1, acpi_device_write_interrupt_or_gpio(ctx, dev,
							   "enable-gpios"));
	ut_asserteq(ACPI_DESCRIPTOR_GPIO, ptr[0]);

	/* This one has neither */
	ptr = acpigen_get_current(ctx);
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 3, &dev));
	ut_asserteq_str("b-test", dev->name);
	ut_asserteq(-ENOENT,
		    acpi_device_write_interrupt_or_gpio(ctx, dev,
							"enable-gpios"));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_interrupt_or_gpio,
	DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test emitting an I2C descriptor */
static int dm_test_acpi_i2c(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev));
	ut_asserteq(0x43, acpi_device_write_i2c_dev(ctx, dev));
	ut_asserteq(28, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_SERIAL_BUS, ptr[0]);
	ut_asserteq(25, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_I2C_SERIAL_BUS_REVISION_ID, ptr[3]);
	ut_asserteq(0, ptr[4]);
	ut_asserteq(ACPI_SERIAL_BUS_TYPE_I2C, ptr[5]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_I2C_TYPE_SPECIFIC_REVISION_ID, ptr[9]);
	ut_asserteq(6, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(100000, get_unaligned((u32 *)(ptr + 12)));
	ut_asserteq(0x43, get_unaligned((u16 *)(ptr + 16)));
	ut_asserteq_str("\\_SB.I2C0", (char *)ptr + 18);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_i2c, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test emitting a SPI descriptor */
static int dm_test_acpi_spi(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(uclass_first_device_err(UCLASS_SPI_FLASH, &dev));
	ut_assertok(acpi_device_write_spi_dev(ctx, dev));
	ut_asserteq(31, acpigen_get_current(ctx) - ptr);
	ut_asserteq(ACPI_DESCRIPTOR_SERIAL_BUS, ptr[0]);
	ut_asserteq(28, get_unaligned((u16 *)(ptr + 1)));
	ut_asserteq(ACPI_SPI_SERIAL_BUS_REVISION_ID, ptr[3]);
	ut_asserteq(0, ptr[4]);
	ut_asserteq(ACPI_SERIAL_BUS_TYPE_SPI, ptr[5]);
	ut_asserteq(2, ptr[6]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 7)));
	ut_asserteq(ACPI_SPI_TYPE_SPECIFIC_REVISION_ID, ptr[9]);
	ut_asserteq(9, get_unaligned((u16 *)(ptr + 10)));
	ut_asserteq(40000000, get_unaligned((u32 *)(ptr + 12)));
	ut_asserteq(8, ptr[16]);
	ut_asserteq(0, ptr[17]);
	ut_asserteq(0, ptr[18]);
	ut_asserteq(0, get_unaligned((u16 *)(ptr + 19)));
	ut_asserteq_str("\\_SB.SPI0", (char *)ptr + 21);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_spi, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test emitting a length */
static int dm_test_acpi_len(struct unit_test_state *uts)
{
	const int size = 0xc0000;
	struct acpi_ctx *ctx;
	u8 *ptr;
	int i;

	ut_assertok(acpi_test_alloc_context_size(&ctx, size));

	ptr = acpigen_get_current(ctx);

	/* Write a byte and a 3-byte length */
	acpigen_write_len_f(ctx);
	acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(1 + 3, acpi_test_get_length(ptr));

	/* Write 200 bytes so we need two length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 200; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(200 + 3, acpi_test_get_length(ptr));

	/* Write 40KB so we need three length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 40000; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(40000 + 3, acpi_test_get_length(ptr));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_len, 0);

/* Test writing a package */
static int dm_test_acpi_package(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	char *num_elements;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	num_elements = acpigen_write_package(ctx, 3);
	ut_asserteq_ptr(num_elements, ptr + 4);

	/* For ease of testing, just emit a byte, not valid package contents */
	acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(PACKAGE_OP, ptr[0]);
	ut_asserteq(5, acpi_test_get_length(ptr + 1));
	ut_asserteq(3, ptr[4]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_package, 0);

/* Test writing an integer */
static int dm_test_acpi_integer(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	acpigen_write_integer(ctx, 0);
	acpigen_write_integer(ctx, 1);
	acpigen_write_integer(ctx, TEST_INT8);
	acpigen_write_integer(ctx, TEST_INT16);
	acpigen_write_integer(ctx, TEST_INT32);
	acpigen_write_integer(ctx, TEST_INT64);

	ut_asserteq(6 + 1 + 2 + 4 + 8, acpigen_get_current(ctx) - ptr);

	ut_asserteq(ZERO_OP, ptr[0]);

	ut_asserteq(ONE_OP, ptr[1]);

	ut_asserteq(BYTE_PREFIX, ptr[2]);
	ut_asserteq(TEST_INT8, ptr[3]);

	ut_asserteq(WORD_PREFIX, ptr[4]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 5)));

	ut_asserteq(DWORD_PREFIX, ptr[7]);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)(ptr + 8)));

	ut_asserteq(QWORD_PREFIX, ptr[12]);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 13)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_integer, 0);

/* Test writing a string */
static int dm_test_acpi_string(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	acpigen_write_string(ctx, TEST_STRING);
	acpigen_write_string(ctx, TEST_STRING2);

	ut_asserteq(2 + sizeof(TEST_STRING) + sizeof(TEST_STRING2),
		    acpigen_get_current(ctx) - ptr);
	ut_asserteq(STRING_PREFIX, ptr[0]);
	ut_asserteq_str(TEST_STRING, (char *)ptr + 1);
	ptr += 1 + sizeof(TEST_STRING);
	ut_asserteq(STRING_PREFIX, ptr[0]);
	ut_asserteq_str(TEST_STRING2, (char *)ptr + 1);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_string, 0);

/* Test writing a name */
static int dm_test_acpi_name(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/*
	 * The names here are made up for testing the various cases. The
	 * grammar is in the ACPI spec 6.3 section 19.2.2
	 */
	acpigen_write_name(ctx, "\\_SB");
	acpigen_write_name(ctx, "\\_SB.I2C0");
	acpigen_write_name(ctx, "\\_SB.I2C0.TPM2");
	acpigen_write_name(ctx, "\\_SB.I2C0.TPM2.LONG");
	acpigen_write_name(ctx, "^^^^SPI0.FLAS");
	acpigen_write_name(ctx, "NN");
	acpigen_write_name(ctx, "^AB.CD.D.EFG");
	acpigen_write_name(ctx, "^^^^");
	acpigen_write_name(ctx, "\\");
	acpigen_write_name(ctx, "\\ABCD");

	ut_asserteq(107, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("\\_SB_", (char *)ptr + 1);
	ptr += 6;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(DUAL_NAME_PREFIX, ptr[2]);
	ut_asserteq_strn("_SB_I2C0", (char *)ptr + 3);
	ptr += 11;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(3, ptr[3]);
	ut_asserteq_strn("_SB_I2C0TPM2", (char *)ptr + 4);
	ptr += 16;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(4, ptr[3]);
	ut_asserteq_strn("_SB_I2C0TPM2LONG", (char *)ptr + 4);
	ptr += 20;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq('^', ptr[2]);
	ut_asserteq('^', ptr[3]);
	ut_asserteq('^', ptr[4]);
	ut_asserteq(DUAL_NAME_PREFIX, ptr[5]);
	ut_asserteq_strn("SPI0FLAS", (char *)ptr + 6);
	ptr += 14;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("NN__", (char *)ptr + 1);
	ptr += 5;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[2]);
	ut_asserteq(4, ptr[3]);
	ut_asserteq_strn("AB__CD__D___EFG_", (char *)ptr + 4);
	ptr += 20;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('^', ptr[1]);
	ut_asserteq('^', ptr[2]);
	ut_asserteq('^', ptr[3]);
	ut_asserteq('^', ptr[4]);
	ut_asserteq(ZERO_OP, ptr[5]);
	ptr += 6;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq('\\', ptr[1]);
	ut_asserteq(ZERO_OP, ptr[2]);
	ptr += 3;

	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn("\\ABCD", (char *)ptr + 1);
	ptr += 5;

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_name, 0);

/* Test writing a UUID */
static int dm_test_acpi_uuid(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	ut_assertok(acpigen_write_uuid(ctx,
				       "dbb8e3e6-5886-4ba6-8795-1319f52a966b"));
	ut_asserteq(23, acpigen_get_current(ctx) - ptr);
	ut_asserteq(BUFFER_OP, ptr[0]);
	ut_asserteq(22, acpi_test_get_length(ptr + 1));
	ut_asserteq(0xdbb8e3e6, get_unaligned((u32 *)(ptr + 7)));
	ut_asserteq(0x5886, get_unaligned((u16 *)(ptr + 11)));
	ut_asserteq(0x4ba6, get_unaligned((u16 *)(ptr + 13)));
	ut_asserteq(0x9587, get_unaligned((u16 *)(ptr + 15)));
	ut_asserteq(0x2af51913, get_unaligned((u32 *)(ptr + 17)));
	ut_asserteq(0x6b96, get_unaligned((u16 *)(ptr + 21)));

	/* Try a bad UUID */
	ut_asserteq(-EINVAL,
		    acpigen_write_uuid(ctx,
				       "dbb8e3e6-5886-4ba6x8795-1319f52a966b"));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_uuid, 0);

/* Test writing misc ACPI codes */
static int dm_test_acpi_misc(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	const int flags = 3;
	const int nargs = 4;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);
	acpigen_write_sleep(ctx, TEST_INT64);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 3)));
	ptr += 11;

	acpigen_write_store(ctx);
	ut_asserteq(STORE_OP, *ptr);
	ptr++;

	acpigen_write_debug_string(ctx, TEST_STRING);
	ut_asserteq_str(TEST_STRING, (char *)ptr + 2);
	ptr += 2 +  sizeof(TEST_STRING);
	ut_asserteq(EXT_OP_PREFIX, ptr[0]);
	ut_asserteq(DEBUG_OP, ptr[1]);
	ptr += 2;

	acpigen_write_sta(ctx, flags);
	ut_asserteq(METHOD_OP, ptr[0]);
	ut_asserteq(11, acpi_test_get_length(ptr + 1));
	ut_asserteq_strn("_STA", (char *)ptr + 4);
	ut_asserteq(0, ptr[8]);
	ut_asserteq(RETURN_OP, ptr[9]);
	ut_asserteq(BYTE_PREFIX, ptr[10]);
	ut_asserteq(flags, ptr[11]);
	ptr += 12;

	acpigen_write_sleep(ctx, TEST_INT16);
	ut_asserteq(SLEEP_OP, ptr[1]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 3)));
	ptr += 5;

	acpigen_write_method_serialized(ctx, "FRED", nargs);
	ut_asserteq(METHOD_OP, ptr[0]);
	ut_asserteq_strn("FRED", (char *)ptr + 4);
	ut_asserteq(1 << 3 | nargs, ptr[8]);
	ut_asserteq(1, ctx->ltop);	/* method is unfinished */

	ptr += 9;
	acpigen_write_or(ctx, LOCAL0_OP, LOCAL1_OP, LOCAL2_OP);
	acpigen_write_and(ctx, LOCAL3_OP, LOCAL4_OP, LOCAL5_OP);
	acpigen_write_not(ctx, LOCAL6_OP, LOCAL7_OP);
	ut_asserteq(OR_OP, ptr[0]);
	ut_asserteq(LOCAL0_OP, ptr[1]);
	ut_asserteq(LOCAL1_OP, ptr[2]);
	ut_asserteq(LOCAL2_OP, ptr[3]);

	ptr += 4;
	ut_asserteq(AND_OP, ptr[0]);
	ut_asserteq(LOCAL3_OP, ptr[1]);
	ut_asserteq(LOCAL4_OP, ptr[2]);
	ut_asserteq(LOCAL5_OP, ptr[3]);

	ptr += 4;
	ut_asserteq(NOT_OP, ptr[0]);
	ut_asserteq(LOCAL6_OP, ptr[1]);
	ut_asserteq(LOCAL7_OP, ptr[2]);
	ptr += 3;
	ut_asserteq_ptr(ptr, ctx->current);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_misc, 0);

/* Test writing an ACPI power resource */
static int dm_test_acpi_power_res(struct unit_test_state *uts)
{
	const char *const states[] = { "_PR0", "_PR3" };
	const char *name = "PRIC";
	const int level = 3;
	const int order = 2;
	struct acpi_ctx *ctx;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	ptr = acpigen_get_current(ctx);

	/* PowerResource (PRIC, 0, 0) */
	acpigen_write_power_res(ctx, name, level, order, states,
				ARRAY_SIZE(states));
	ut_asserteq(0x28, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, ptr[0]);
	ut_asserteq_strn(states[0], (char *)ptr + 1);
	ut_asserteq(8, acpi_test_get_length(ptr + 6));
	ut_asserteq_strn(name, (char *)ptr + 0xa);

	ut_asserteq_strn(states[1], (char *)ptr + 0xf);
	ut_asserteq(8, acpi_test_get_length(ptr + 0x14));
	ut_asserteq_strn(name, (char *)ptr + 0x18);

	ut_asserteq(POWER_RES_OP, ptr[0x1d]);
	ut_asserteq_strn(name, (char *)ptr + 0x21);
	ut_asserteq(level, ptr[0x25]);
	ut_asserteq(order, get_unaligned((u16 *)(ptr + 0x26)));

	/* The length is not set - caller must use acpigen_pop_len() */
	ut_asserteq(1, ctx->ltop);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_power_res, 0);

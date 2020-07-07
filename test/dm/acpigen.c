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

/* Maximum size of the ACPI context needed for most tests */
#define ACPI_CONTEXT_SIZE	150

#define TEST_STRING	"frogmore"
#define TEST_STREAM2	"\xfa\xde"

#define TEST_INT8	0x7d
#define TEST_INT16	0x2345
#define TEST_INT32	0x12345678
#define TEST_INT64	0x4567890123456

static int alloc_context_size(struct acpi_ctx **ctxp, int size)
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

static int alloc_context(struct acpi_ctx **ctxp)
{
	return alloc_context_size(ctxp, ACPI_CONTEXT_SIZE);
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

/**
 * get_length() - decode a three-byte length field
 *
 * @ptr: Length encoded as per ACPI
 * @return decoded length, or -EINVAL on error
 */
static int get_length(u8 *ptr)
{
	if (!(*ptr & 0x80))
		return -EINVAL;

	return (*ptr & 0xf) | ptr[1] << 4 | ptr[2] << 12;
}

/* Test emitting a length */
static int dm_test_acpi_len(struct unit_test_state *uts)
{
	const int size = 0xc0000;
	struct acpi_ctx *ctx;
	u8 *ptr;
	int i;

	ut_assertok(alloc_context_size(&ctx, size));

	ptr = acpigen_get_current(ctx);

	/* Write a byte and a 3-byte length */
	acpigen_write_len_f(ctx);
	acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(1 + 3, get_length(ptr));

	/* Write 200 bytes so we need two length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 200; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(200 + 3, get_length(ptr));

	/* Write 40KB so we need three length bytes */
	ptr = ctx->current;
	acpigen_write_len_f(ctx);
	for (i = 0; i < 40000; i++)
		acpigen_emit_byte(ctx, 0x23);
	acpigen_pop_len(ctx);
	ut_asserteq(40000 + 3, get_length(ptr));

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
	ut_asserteq(5, get_length(ptr + 1));
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

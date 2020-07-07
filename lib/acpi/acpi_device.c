// SPDX-License-Identifier: GPL-2.0
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <log.h>
#include <acpi/acpi_device.h>
#include <acpi/acpigen.h>
#include <dm/acpi.h>

/**
 * acpi_device_path_fill() - Find the root device and build a path from there
 *
 * This recursively reaches back to the root device and progressively adds path
 * elements until the device is reached.
 *
 * @dev: Device to return path of
 * @buf: Buffer to hold the path
 * @buf_len: Length of buffer
 * @cur: Current position in the buffer
 * @return new position in buffer after adding @dev, or -ve on error
 */
static int acpi_device_path_fill(const struct udevice *dev, char *buf,
				 size_t buf_len, int cur)
{
	char name[ACPI_NAME_MAX];
	int next = 0;
	int ret;

	ret = acpi_get_name(dev, name);
	if (ret)
		return ret;

	/*
	 * Make sure this name segment will fit, including the path segment
	 * separator and possible NULL terminator, if this is the last segment.
	 */
	if (cur + strlen(name) + 2 > buf_len)
		return -ENOSPC;

	/* Walk up the tree to the root device */
	if (dev_get_parent(dev)) {
		next = acpi_device_path_fill(dev_get_parent(dev), buf, buf_len,
					     cur);
		if (next < 0)
			return next;
	}

	/* Fill in the path from the root device */
	next += snprintf(buf + next, buf_len - next, "%s%s",
			 dev_get_parent(dev) && *name ? "." : "", name);

	return next;
}

int acpi_device_path(const struct udevice *dev, char *buf, int maxlen)
{
	int ret;

	ret = acpi_device_path_fill(dev, buf, maxlen, 0);
	if (ret < 0)
		return ret;

	return 0;
}

int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen)
{
	int ret;

	if (!dev_get_parent(dev))
		return log_msg_ret("noparent", -EINVAL);

	ret = acpi_device_path_fill(dev_get_parent(dev), scope, maxlen, 0);
	if (ret < 0)
		return log_msg_ret("fill", ret);

	return 0;
}

enum acpi_dev_status acpi_device_status(const struct udevice *dev)
{
	return ACPI_DSTATUS_ALL_ON;
}

/**
 * largeres_write_len_f() - Write a placeholder word value
 *
 * Write a forward length for a large resource (2 bytes)
 *
 * @return pointer to the zero word (for fixing up later)
 */
static void *largeres_write_len_f(struct acpi_ctx *ctx)
{
	u8 *p = acpigen_get_current(ctx);

	acpigen_emit_word(ctx, 0);

	return p;
}

/**
 * largeres_fill_from_len() - Fill in a length value
 *
 * This calculated the number of bytes since the provided @start and writes it
 * to @ptr, which was previous returned by largeres_write_len_f().
 *
 * @ptr: Word to update
 * @start: Start address to count from to calculated the length
 */
static void largeres_fill_from_len(struct acpi_ctx *ctx, char *ptr, u8 *start)
{
	u16 len = acpigen_get_current(ctx) - start;

	ptr[0] = len & 0xff;
	ptr[1] = (len >> 8) & 0xff;
}

/**
 * largeres_fill_len() - Fill in a length value, excluding the length itself
 *
 * Fill in the length field with the value calculated from after the 16bit
 * field to acpigen current. This is useful since the length value does not
 * include the length field itself.
 *
 * This calls acpi_device_largeres_fill_len() passing @ptr + 2 as @start
 *
 * @ptr: Word to update.
 */
static void largeres_fill_len(struct acpi_ctx *ctx, void *ptr)
{
	largeres_fill_from_len(ctx, ptr, ptr + sizeof(u16));
}

/* ACPI 6.3 section 6.4.3.6: Extended Interrupt Descriptor */
static int acpi_device_write_interrupt(struct acpi_ctx *ctx,
				       const struct acpi_irq *irq)
{
	void *desc_length;
	u8 flags;

	if (!irq->pin)
		return -ENOENT;

	/* This is supported by GpioInt() but not Interrupt() */
	if (irq->polarity == ACPI_IRQ_ACTIVE_BOTH)
		return -EINVAL;

	/* Byte 0: Descriptor Type */
	acpigen_emit_byte(ctx, ACPI_DESCRIPTOR_INTERRUPT);

	/* Byte 1-2: Length (filled in later) */
	desc_length = largeres_write_len_f(ctx);

	/*
	 * Byte 3: Flags
	 *  [7:5]: Reserved
	 *    [4]: Wake     (0=NO_WAKE   1=WAKE)
	 *    [3]: Sharing  (0=EXCLUSIVE 1=SHARED)
	 *    [2]: Polarity (0=HIGH      1=LOW)
	 *    [1]: Mode     (0=LEVEL     1=EDGE)
	 *    [0]: Resource (0=PRODUCER  1=CONSUMER)
	 */
	flags = BIT(0); /* ResourceConsumer */
	if (irq->mode == ACPI_IRQ_EDGE_TRIGGERED)
		flags |= BIT(1);
	if (irq->polarity == ACPI_IRQ_ACTIVE_LOW)
		flags |= BIT(2);
	if (irq->shared == ACPI_IRQ_SHARED)
		flags |= BIT(3);
	if (irq->wake == ACPI_IRQ_WAKE)
		flags |= BIT(4);
	acpigen_emit_byte(ctx, flags);

	/* Byte 4: Interrupt Table Entry Count */
	acpigen_emit_byte(ctx, 1);

	/* Byte 5-8: Interrupt Number */
	acpigen_emit_dword(ctx, irq->pin);

	/* Fill in Descriptor Length (account for len word) */
	largeres_fill_len(ctx, desc_length);

	return 0;
}

int acpi_device_write_interrupt_irq(struct acpi_ctx *ctx,
				    const struct irq *req_irq)
{
	struct acpi_irq irq;
	int ret;

	ret = irq_get_acpi(req_irq, &irq);
	if (ret)
		return log_msg_ret("get", ret);
	ret = acpi_device_write_interrupt(ctx, &irq);
	if (ret)
		return log_msg_ret("write", ret);

	return 0;
}

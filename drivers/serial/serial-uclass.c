/*
 * Copyright (c) 2014 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <os.h>
#include <serial.h>
#include <stdio_dev.h>
#include <dm/lists.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

/* The currently-selected console serial device */
struct udevice *cur_dev __attribute__ ((section(".data")));

#ifndef CONFIG_SYS_MALLOC_F_LEN
#error "Serial is required before relocation - define CONFIG_SYS_MALLOC_F_LEN to make this work"
#endif

static void serial_find_console_or_panic(void)
{
#ifdef CONFIG_OF_CONTROL
	int node;

	/* Check for a chosen console */
	node = fdtdec_get_chosen_node(gd->fdt_blob, "stdout-path");
	if (node < 0)
		node = fdtdec_get_alias_node(gd->fdt_blob, "console");
	if (!uclass_get_device_by_of_offset(UCLASS_SERIAL, node, &cur_dev))
		return;

	/*
	 * If the console is not marked to be bound before relocation, bind
	 * it anyway.
	 */
	if (node > 0 &&
	    !lists_bind_fdt(gd->dm_root, gd->fdt_blob, node, &cur_dev)) {
		if (!device_probe(cur_dev))
			return;
		cur_dev = NULL;
	}
#endif
	/*
	 * Failing that, get the device with sequence number 0, or in extremis
	 * just the first serial device we can find. But we insist on having
	 * a console (even if it is silent).
	 */
	if (uclass_get_device_by_seq(UCLASS_SERIAL, 0, &cur_dev) &&
	    (uclass_first_device(UCLASS_SERIAL, &cur_dev) || !cur_dev))
		panic("No serial driver found");
}

/* Called prior to relocation */
int serial_init(void)
{
	serial_find_console_or_panic();
	gd->flags |= GD_FLG_SERIAL_READY;

	return 0;
}

/* Called after relocation */
void serial_initialize(void)
{
	serial_find_console_or_panic();
}

void serial_putc(char ch)
{
	struct dm_serial_ops *ops = serial_get_ops(cur_dev);
	int err;

	do {
		err = ops->putc(cur_dev, ch);
	} while (err == -EAGAIN);
	if (ch == '\n')
		serial_putc('\r');
}

void serial_setbrg(void)
{
	struct dm_serial_ops *ops = serial_get_ops(cur_dev);

	if (ops->setbrg)
		ops->setbrg(cur_dev, gd->baudrate);
}

void serial_puts(const char *str)
{
	while (*str)
		serial_putc(*str++);
}

int serial_tstc(void)
{
	struct dm_serial_ops *ops = serial_get_ops(cur_dev);

	if (ops->pending)
		return ops->pending(cur_dev, true);

	return 1;
}

int serial_getc(void)
{
	struct dm_serial_ops *ops = serial_get_ops(cur_dev);
	int err;

	do {
		err = ops->getc(cur_dev);
	} while (err == -EAGAIN);

	return err >= 0 ? err : 0;
}

void serial_stdio_init(void)
{
}

void serial_stub_putc(struct stdio_dev *sdev, const char ch)
{
	struct udevice *dev = sdev->priv;
	struct dm_serial_ops *ops = serial_get_ops(dev);

	ops->putc(dev, ch);
}

void serial_stub_puts(struct stdio_dev *sdev, const char *str)
{
	while (*str)
		serial_stub_putc(sdev, *str++);
}

int serial_stub_getc(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct dm_serial_ops *ops = serial_get_ops(dev);

	int err;

	do {
		err = ops->getc(dev);
	} while (err == -EAGAIN);

	return err >= 0 ? err : 0;
}

int serial_stub_tstc(struct stdio_dev *sdev)
{
	struct udevice *dev = sdev->priv;
	struct dm_serial_ops *ops = serial_get_ops(dev);

	if (ops->pending)
		return ops->pending(dev, true);

	return 1;
}

static int serial_post_probe(struct udevice *dev)
{
	struct stdio_dev sdev;
	struct dm_serial_ops *ops = serial_get_ops(dev);
	struct serial_dev_priv *upriv = dev->uclass_priv;
	int ret;

	/* Set the baud rate */
	if (ops->setbrg) {
		ret = ops->setbrg(dev, gd->baudrate);
		if (ret)
			return ret;
	}

	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	memset(&sdev, '\0', sizeof(sdev));

	strncpy(sdev.name, dev->name, sizeof(sdev.name));
	sdev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	sdev.priv = dev;
	sdev.putc = serial_stub_putc;
	sdev.puts = serial_stub_puts;
	sdev.getc = serial_stub_getc;
	sdev.tstc = serial_stub_tstc;
	stdio_register_dev(&sdev, &upriv->sdev);

	return 0;
}

static int serial_pre_remove(struct udevice *dev)
{
#ifdef CONFIG_SYS_STDIO_DEREGISTER
	struct serial_dev_priv *upriv = dev->uclass_priv;

	if (stdio_deregister_dev(upriv->sdev, 0))
		return -EPERM;
#endif

	return 0;
}

UCLASS_DRIVER(serial) = {
	.id		= UCLASS_SERIAL,
	.name		= "serial",
	.post_probe	= serial_post_probe,
	.pre_remove	= serial_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct serial_dev_priv),
};

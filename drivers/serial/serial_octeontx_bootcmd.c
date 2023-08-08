/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <watchdog.h>
#include <stdio_dev.h>
#include <input.h>

#undef CONFIG_LOGLEVEL
#define CONFIG_LOGLEVEL 10

DECLARE_GLOBAL_DATA_PTR;

#define OCTEONTX_BOOTCMD_BUF_SIZE		4096

/** "BOOTCMD!" */
#define OCTEONTX_PIC_IO_BUF_MAGIC		0x21444d43544f4f42
#define OCTEONTX_PCI_IO_BUF_OWNER_INVALID	0
#define OCTEONTX_PCI_IO_BUF_OWNER_OCTEONTX	1
#define OCTEONTX_PCI_IO_BUF_OWNER_HOST		2

#define DRIVER_NAME				"pci-bootcmd"

#ifndef CONFIG_DM_SERIAL
# error CONFIG_DM_SERIAL required
#endif

struct octeontx_pci_io_buf {
	u64 magic;
	u32 owner;
	u32 len;
	char data[0];
};

struct octeontx_bootcmd_platdata {
	fdt_addr_t base;
	fdt_size_t size;
};

struct octeontx_bootcmd_data {
	struct octeontx_pci_io_buf *buf;
	struct udevice *dev;
	fdt_size_t size;	/** Size of buffer descriptor & data */
	unsigned long data_size;/** Size of buffer data */
	u32 copy_offset;
	bool eol;
	bool started;
	bool unlocked;
};

static struct udevice *bootcmd_dev;

static int octeontx_bootcmd_start(struct udevice *dev)
{
	struct octeontx_bootcmd_data *bc = dev_get_priv(dev);
	struct octeontx_pci_io_buf *buf = bc->buf;
	struct octeontx_bootcmd_platdata *plat = dev_get_platdata(bc->dev);

	if (bc->started) {
		debug("%s: Already started\n", __func__);
		return 0;
	}

	dev_dbg(bc->dev, "%s(%s)\n", __func__, dev->name);
	/* Get address of IO buffer and check it */
	bc->size = plat->size;
	bc->data_size = bc->size - sizeof(struct octeontx_pci_io_buf);
	bc->copy_offset = 0;
	bc->eol = false;

	buf->len = 0;
	buf->data[0] = '\0';
	buf->magic = OCTEONTX_PIC_IO_BUF_MAGIC;
	if (bc->unlocked)
		buf->owner = OCTEONTX_PCI_IO_BUF_OWNER_HOST;
	else
		buf->owner = OCTEONTX_PCI_IO_BUF_OWNER_OCTEONTX;

	bc->unlocked = true;
	bc->started = true;
	__iowmb();
	flush_dcache_range((ulong)buf, (ulong)buf + bc->size);

	return 0;
}

/**
 * Return if there are any pending characters for input or output
 * @param	dev	serial device
 * @param	input	true to check for pending input
 *
 * @return	1 if pending data, 0 if no data available
 */
static int octeontx_bootcmd_pending(struct udevice *dev, bool input)
{
	struct octeontx_bootcmd_data *bc = dev_get_priv(dev);
	struct octeontx_pci_io_buf *buf = bc->buf;

	if (!bc->started) {
		dev_dbg("%s: Error: not started\n", __func__);
		return 0;
	}

	invalidate_dcache_range((ulong)buf,
				(ulong)buf + bc->size);
	__iormb();
	if (input) {
		if (bc->eol)
			return 1;
		if (buf->owner != OCTEONTX_PCI_IO_BUF_OWNER_OCTEONTX)
			return 0;
		if ((buf->len > bc->copy_offset) &&
		    (buf->data[bc->copy_offset] != '\0'))
			return 1;
		return 0;
	}

	return 0;
}

static int octeontx_bootcmd_putc(struct udevice *dev, const char ch)
{
	return 0;
}

static int octeontx_bootcmd_getc(struct udevice *dev)
{
	struct octeontx_bootcmd_data *bc = dev_get_priv(dev);
	struct octeontx_pci_io_buf *buf = bc->buf;
	char c;
	int end;

	if (!bc->started) {
		dev_dbg(dev, "%s: Error: start not called\n", __func__);
		return -1;
	}
	/* There's no EOL for boot commands so we fake it. */
	if (bc->eol) {
		bc->eol = false;
		return '\n';
	}

	while (!octeontx_bootcmd_pending(dev, true)) {
		WATCHDOG_RESET();
		udelay(0);
	}

	__iormb();
	c = buf->data[bc->copy_offset];
	buf->data[bc->copy_offset++] = '\0';

	end = bc->data_size < CONFIG_SYS_CBSIZE ?
			bc->data_size - 1 : CONFIG_SYS_CBSIZE - 1;
	if ((bc->copy_offset >= end) || (buf->data[bc->copy_offset] == '\0')) {
		bc->copy_offset = 0;
		buf->len = 0;
		buf->owner = OCTEONTX_PCI_IO_BUF_OWNER_HOST;
		bc->eol = true;
	}
	__iowmb();
	flush_dcache_range((ulong)buf, (ulong)buf + bc->size);

	return c;
}

/**
 * stdio driver getc
 *
 * @param	dev	stdio device
 * @return	character returned from input
 */
static int bootcmd_stdio_getc(struct stdio_dev *dev)
{
	const struct dm_serial_ops *ops;

	if (!bootcmd_dev) {
		printf("%s(%s): Error: bootcmd_dev NULL!\n",
		       __func__, dev->name);
		return -1;
	}
	ops = device_get_ops(bootcmd_dev);
	return ops->getc(bootcmd_dev);
}

/**
 * stdio driver testc
 *
 * @param	dev	stdio device
 *
 * @return	-1 on error, 1 if pending data, 0 if no data
 */
static int bootcmd_stdio_tstc(struct stdio_dev *dev)
{
	const struct dm_serial_ops *ops;

	if (!bootcmd_dev) {
		printf("%s(%s): Error: bootcmd_dev NULL!\n",
		       __func__, dev->name);
		return -1;
	}
	ops = device_get_ops(bootcmd_dev);

	return ops->pending(bootcmd_dev, true);
}

/**
 * Probe function for bootcmd driver
 *
 * @param	dev	serial device
 *
 * @return	0 for success, otherwise error
 */
static int octeontx_bootcmd_probe(struct udevice *dev)
{
	struct octeontx_bootcmd_data *bc;
	struct octeontx_pci_io_buf *buf;
	struct octeontx_bootcmd_platdata *plat = dev_get_platdata(dev);
	struct stdio_dev bdev;
	int ret;

	dev_dbg(dev, "%s(%s)\n", __func__, dev->name);
	bc = dev_get_priv(dev);
	buf = (struct octeontx_pci_io_buf *)(plat->base);
	bc->buf = buf;
	bc->dev = dev;
	bc->unlocked = true;
	dev_dbg(dev, "%s: bootcmd IO buffer: %p\n", __func__, buf);
	ret = octeontx_bootcmd_start(dev);
	if (!ret) {
		bootcmd_dev = dev;
		memset(&bdev, 0, sizeof(bdev));
		snprintf(bdev.name, sizeof(bdev.name), DRIVER_NAME);
		bdev.flags = DEV_FLAGS_INPUT;
		bdev.getc = bootcmd_stdio_getc;
		bdev.tstc = bootcmd_stdio_tstc;
		pr_debug("%s: Registering stdin driver %s from device %s, bootcmd_dev: %p\n",
			 __func__, bdev.name, bootcmd_dev->name, bootcmd_dev);
		ret = stdio_register(&bdev);
		if (ret)
			printf("%s: Error registering stdin device %s\n",
			       __func__, bdev.name);
	}
	return ret;
}

/**
 * Extracts the platform data from the device tree
 *
 * @param	dev	serial device
 *
 * @return	0 for success, otherwise error
 */
static int octeontx_bootcmd_ofdata_to_platdata(struct udevice *dev)
{
	struct octeontx_bootcmd_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = devfdt_get_addr_size_index(dev, 0, &size);
	dev_dbg(dev, "%s(%s): base: 0x%llx, size: 0x%llx\n", __func__,
		dev->name, addr, size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	plat->size = size;
	return 0;
}

static const struct dm_serial_ops octeontx_bootcmd_ops = {
	.putc = octeontx_bootcmd_putc,
	.pending = octeontx_bootcmd_pending,
	.getc = octeontx_bootcmd_getc,
};

static const struct udevice_id octeontx_bootcmd_serial_id[] = {
	{ .compatible = "marvell,pci-bootcmd", },
	{ },
};

U_BOOT_DRIVER(octeontx_bootcmd) = {
	.name	= DRIVER_NAME,
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(octeontx_bootcmd_serial_id),
	.ofdata_to_platdata = of_match_ptr(octeontx_bootcmd_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct octeontx_bootcmd_platdata),
	.probe = octeontx_bootcmd_probe,
	.ops = &octeontx_bootcmd_ops,
	.priv_auto_alloc_size = sizeof(struct octeontx_bootcmd_data),
	.flags = DM_FLAG_PRE_RELOC,
};

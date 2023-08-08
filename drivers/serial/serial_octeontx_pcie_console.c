// SPDX-License-Identifier: GPL-2.0+    BSD-3-Clause
/*
 * Copyright (C) 2019 Marvell International Ltd.
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <linux/ioport.h>
#include <errno.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <watchdog.h>
#include <stdio_dev.h>
#include <input.h>
#include <asm/arch/lock.h>
#include "serial_octeontx_pcie_console.h"
#include <string.h>
#include <iomux.h>

/* This driver provides a PCIe console for OcteonTX processors.  It behaves
 * similar to a serial console but it works by using shared memory between
 * the host controller and the target (this being the target).
 *
 * Multiple consoles are supported and the consoles themselves are designed
 * to be shared.  This design allows for U-Boot and ATF to share a console,
 * for example, however this initial version does not support this sharing.
 *
 * Because the main data structure is shared between ATF, U-Boot and Linux
 * as well as the host, atomic operations are required.  Locks are also
 * required for the data structures if they are shared between "processes."
 *
 * The pcie lock uses Peterson's algorithm to lock between the host and
 * the target.
 *
 * Memory write barriers are also used and are essential to maintain
 * the integrity between the host and target.
 */

#ifndef CONFIG_DM_STDIO
# error CONFIG_DM_STDIO is required!
#endif

#define DRIVER_NAME	"pci-console"

struct driver octeontx_pcie_console_nexus;
struct driver octeontx_pcie_console;

#ifdef DEBUG
bool in_debug;
#endif

/**
 * Returns the number of available bytes in the buffer
 *
 * @param buffer_size	size of buffer
 * @param wr_idx	write index
 * @param rd_idx	read index
 *
 * @return number of bytes free
 */
static int octeontx_pcie_console_buffer_free_bytes(size_t buffer_size,
						   u32 wr_idx, u32 rd_idx)
{
	if (rd_idx >= buffer_size || wr_idx >= buffer_size)
		return -1;
	return ((buffer_size - 1) - (wr_idx - rd_idx)) % buffer_size;
}

static int octeontx_pcie_console_buffer_avail_bytes(size_t buffer_size,
						    u32 wr_idx, u32 rd_idx)
{
	if (rd_idx >= buffer_size || wr_idx >= buffer_size)
		return -1;
	return buffer_size - 1 -
	       octeontx_pcie_console_buffer_free_bytes(buffer_size, wr_idx,
						       rd_idx);
}

/**
 * Check that the console version is acceptable.
 */
static bool octeontx_pcie_console_check_ver(u8 major, u8 minor)
{
	if (major > OCTEONTX_PCIE_CONSOLE_MAJOR)
		return true;
	if (major == OCTEONTX_PCIE_CONSOLE_MAJOR &&
	    minor >= OCTEONTX_PCIE_CONSOLE_MINOR)
		return true;
	return false;
}

/**
 * Clears bytes from the output buffer if the host console is not connected
 *
 * @param console		console to clear output from
 * @param bytes_to_clear	Number of bytes to free up
 *
 * @return	0 for success, -1 on error.  If positive, it returns
 *		the amount of available space that is less than
 *		bytes_to_clear.
 */
int octeontx_pcie_console_output_trunc(struct octeontx_pcie_console *console,
				       size_t bytes_to_clear)
{
	u64 old_val;
	u64 new_val;
	size_t bytes_avail;
	const u32 out_buf_size = le32_to_cpu(console->output_buf_size);
	u32 out_wr_idx = le32_to_cpu(console->output_write_index);
	u32 out_rd_idx = le32_to_cpu(console->output_read_index);
	int ret;

	if (console->host_console_connected)
		return -1;

	old_val = cpu_to_le64((u64)out_rd_idx << 32);
	bytes_avail = octeontx_pcie_console_buffer_avail_bytes(out_buf_size,
							       out_wr_idx,
							       out_rd_idx);
	if (bytes_avail < 0)
		return bytes_avail;
	/* Not enough space */
	if (bytes_to_clear > bytes_avail)
		return bytes_avail;

	out_rd_idx = (out_rd_idx + bytes_to_clear) % out_buf_size;
	new_val = cpu_to_le64((u64)out_rd_idx << 32);

	/*
	 * We need to use an atomic operation here in case the host
	 * console should connect.  This guarantees that if the host
	 * connects that it will always see a consistent state.  Normally
	 * only the host can modify the read pointer.  This assures us
	 * that the read pointer will only be modified if the host
	 * is disconnected.
	 */
	ret = __atomic_compare_exchange_n
			((u64 *)(&console->host_console_connected),
			 &old_val, new_val, 0,
			 __ATOMIC_RELAXED, __ATOMIC_RELAXED);

	return ret ? 0 : -1;
}

int octeontx_pcie_console_write(struct octeontx_pcie_console *console,
				const u8 *buffer, size_t bytes_to_write,
				u32 flags)
{
	u8 *buf_ptr;
	size_t bytes_available;
	size_t bytes_written;
	int ret;

	buf_ptr = (u8 *)le64_to_cpu(console->output_base_addr);
	bytes_written = 0;

	while (bytes_to_write > 0) {
		u32 out_size = le32_to_cpu(console->output_buf_size);
		u32 out_wr_idx = le32_to_cpu(console->output_write_index);
		u32 out_rd_idx = le32_to_cpu(console->output_read_index);

		bytes_available =
			octeontx_pcie_console_buffer_free_bytes(out_size,
								out_wr_idx,
								out_rd_idx);
		if (bytes_available > 0) {
			int write_size = min(bytes_available, bytes_to_write);

			if (out_wr_idx + write_size >= out_size)
				write_size = out_size - out_wr_idx;
			memcpy(buf_ptr + out_wr_idx,
			       buffer + bytes_written, write_size);
			__iowmb();
			console->output_write_index =  cpu_to_le32
					((out_wr_idx + write_size) % out_size);
			__iowmb();
			bytes_to_write -= write_size;
			bytes_written += write_size;
		} else if (bytes_available == 0) {
			/* Check to see if we should wait for room, or return
			 * after partial write
			 */
			if (!(flags & OCTEONTX_PCIE_CONSOLE_FLAG_NONBLOCK))
				continue;

			ret = octeontx_pcie_console_output_trunc(console,
							bytes_to_write);
			if (ret < 0) {
				mdelay(1);
			} else if (ret > 0) {
				octeontx_pcie_console_output_trunc(console,
								   ret);
				mdelay(1);
			}
		} else {
			bytes_written = -1;
			goto done;
		}
	}
done:
	return bytes_written;
}

static bool
octeontx_pcie_console_input_empty(struct octeontx_pcie_console *console)
{
	/* endian conversion is not needed */
	return console->input_read_index == console->input_write_index;
}

static int octeontx_pcie_console_readc(struct octeontx_pcie_console *console,
				       u32 flags)
{
	const u32 in_buf_size = le32_to_cpu(console->input_buf_size);
	u32 in_rd_idx = le32_to_cpu(console->input_read_index);
	u8 *buf_ptr = (u8 *)le64_to_cpu(console->input_base_addr);
	int ret;

	assert(in_rd_idx < in_buf_size);

	if (octeontx_pcie_console_input_empty(console)) {
		debug("input empty: rd_idx: %#x\n", in_rd_idx);
		return -1;
	}

	ret = buf_ptr[in_rd_idx++];
	if (in_rd_idx >= in_buf_size)
		in_rd_idx = 0;
	console->input_read_index = cpu_to_le32(in_rd_idx);
	assert(le32_to_cpu(console->input_write_index) < in_buf_size);
	__iowmb();

	return ret;
}

static int octeontx_pcie_console_read_avail
				(const struct octeontx_pcie_console *console)
{
	int bytes_available;
	u32 in_size = le32_to_cpu(console->input_buf_size);
	u32 in_wr_idx = le32_to_cpu(console->input_write_index);
	u32 in_rd_idx = le32_to_cpu(console->input_read_index);

	bytes_available =
		octeontx_pcie_console_buffer_avail_bytes(in_size, in_wr_idx,
							 in_rd_idx);
	assert(bytes_available < in_size);
	return (bytes_available >= 0) ? bytes_available : 0;
}

static int octeontx_pcie_console_write_avail
				(const struct octeontx_pcie_console *console)
{
	int bytes_available;
	u32 out_size = le32_to_cpu(console->output_buf_size);
	u32 out_wr_idx = le32_to_cpu(console->output_write_index);
	u32 out_rd_idx = le32_to_cpu(console->output_read_index);

	bytes_available =
		octeontx_pcie_console_buffer_free_bytes(out_size, out_wr_idx,
							out_rd_idx);
	assert(bytes_available < out_size);
	return (bytes_available >= 0) ? bytes_available : 0;
}

static int octeontx_pcie_console_clear_rx(struct octeontx_pcie_console *console)
{
	/* No need for endian conversion */
	console->input_read_index = console->input_write_index;
	__iowmb();
	return 0;
}

static int octeontx_pcie_console_stdio_start(struct stdio_dev *dev)
{
	debug("%s(%s)\n", __func__, dev->name);
	return 0;
}

/**
 * Removes the console
 *
 * @param	dev	stdio device
 *
 * @return	0 for success, -EINVAL if the console descriptor is corrupt
 */
static int octeontx_pcie_console_stdio_stop(struct stdio_dev *dev)
{
	struct udevice *udev = dev->priv;
	struct octeontx_pcie_console_priv *priv = dev_get_priv(udev);
	struct octeontx_pcie_console *console = priv->console;
	struct octeontx_pcie_console_nexus *desc = priv->nexus;
	u32 old_in_use;
	u64 old_val;
	u64 mask = (1 << priv->console_num);

	debug("%s(%s)\n", __func__, dev->name);
	if (!(le32_to_cpu(desc->exclusive) & 1 << priv->console_num))
		if (le32_to_cpu(console->owner_id) !=
					OCTEONTX_PCIE_CONSOLE_OWNER_UBOOT) {
			dev_dbg(dev,
				"Console %d is shared, not shutting it down\n",
				priv->console_num);
			return 0;
		}

	console->owner_id = cpu_to_le32(OCTEONTX_PCIE_CONSOLE_OWNER_UNUSED);

	mask |= mask << 32;
	debug("%s(%s): mask: %#llx\n", __func__, dev->name, mask);
	/* Atomically remove ourselves from the nexus descriptor */
#ifdef __LITTLE_ENDIAN
	old_val = __atomic_fetch_nand((u64 *)&(desc->in_use), mask,
				      __ATOMIC_SEQ_CST);
#else
	old_val = __atomic_fetch_nand((u64 *)&(desc->exclusive), mask,
				      __ATOMIC_SEQ_CST);
#endif
	old_val = le64_to_cpu(old_val);
	old_in_use = lower_32_bits(old_val);

	if (!(old_in_use & mask)) {
		dev_err(udev,
			"Error: console %d not in descriptor usage mask %#x\n",
			priv->console_num, old_in_use);
		return -EINVAL;
	}

	return 0;
}

static int octeontx_pcie_console_stdio_getc(struct stdio_dev *dev)
{
	struct udevice *udev = dev->priv;
	struct octeontx_pcie_console_priv *priv = dev_get_priv(udev);
	struct octeontx_pcie_console *console = priv->console;

	return octeontx_pcie_console_readc(console, 0);
}

static int octeontx_pcie_console_stdio_tstc(struct stdio_dev *dev)
{
	struct udevice *udev = dev->priv;
	struct octeontx_pcie_console_priv *priv = dev_get_priv(udev);
	struct octeontx_pcie_console *console = priv->console;

	return !octeontx_pcie_console_input_empty(console);
}

static void octeontx_pcie_console_stdio_putc(struct stdio_dev *dev,
					     const char c)
{
	struct udevice *udev = dev->priv;
	struct octeontx_pcie_console_priv *priv = dev_get_priv(udev);
	struct octeontx_pcie_console *console = priv->console;
	u8 v = c;

#ifdef DEBUG
	if (in_debug)
		return;
#endif
	octeontx_pcie_console_write(console, &v, 1,
				    OCTEONTX_PCIE_CONSOLE_FLAG_NONBLOCK);
}

static void octeontx_pcie_console_stdio_puts(struct stdio_dev *dev,
					     const char *s)
{
	struct udevice *udev = dev->priv;
	size_t len = strlen(s);
	struct octeontx_pcie_console_priv *priv = dev_get_priv(udev);
	struct octeontx_pcie_console *console = priv->console;

#ifdef DEBUG
	if (in_debug)
		return;
#endif
	octeontx_pcie_console_write(console, (u8 *)s, len,
				    OCTEONTX_PCIE_CONSOLE_FLAG_NONBLOCK);
}

static int octeontx_pcie_console_setbrg(struct udevice *dev, int baudrate)
{
	return 0;
}

static int octeontx_pcie_console_getc(struct udevice *dev)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *console = priv->console;
	int ret;

#ifdef DEBUG
	if (in_debug)
		return 0;
#endif
	ret = octeontx_pcie_console_readc(console, 0);
	debug("%s(%s): ret=%d\n", __func__, dev->name, ret);
	return ret;
}

static int octeontx_pcie_console_putc(struct udevice *dev, const char c)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *console = priv->console;
	int ret;
	u8 v = (u8)c;

#ifdef DEBUG
	if (in_debug)
		return 0;
#endif
	ret = octeontx_pcie_console_write(console, &v, 1,
					  OCTEONTX_PCIE_CONSOLE_FLAG_NONBLOCK);
	return ret > 0 ? 0 : -1;
}

static int octeontx_pcie_console_pending(struct udevice *dev, bool input)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *console = priv->console;
	bool ret;

	if (input)
		ret = octeontx_pcie_console_read_avail(console);
	else
		ret = octeontx_pcie_console_write_avail(console);

	return ret;
}

static int octeontx_pcie_console_clear(struct udevice *dev)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *console = priv->console;
	int ret;

#ifdef DEBUG
	if (in_debug)
		return 0;
#endif
	/* We cannot cleanly clear the write buffer */
	ret = octeontx_pcie_console_clear_rx(console);
	if (ret < 0)
		return ret;
	if (octeontx_pcie_console_write_avail(console) > 0)
		return -EAGAIN;
	return 0;
}

/**
 * Initialize the nexus console descriptor
 *
 * If it is not already initialized, this parses the device tree and
 * initializes the nexus console descriptor.  We need it to point to
 * all of the consoles since the remote console utility does not have
 * access to the device tree.
 *
 * Note that this tries to initialize the magic number and version atomically.
 *
 * The order this is written tries to minimize any locking needed
 * or atomic operations.  It should be safe for multiple clients to try and
 * initialize this at the same time.
 */
int octeontx_pcie_console_available(struct udevice *ndev, int console_num,
				    bool *in_use, bool *exclusive)
{
	struct octeontx_pcie_console_nexus_priv *npriv = dev_get_priv(ndev);
	struct octeontx_pcie_console_nexus *cdesc = npriv->nexus;
	bool used;

	if ((le64_to_cpu(cdesc->magic) != OCTEONTX_PCIE_CONSOLE_NEXUS_MAGIC) ||
	    !octeontx_pcie_console_check_ver(cdesc->major_version,
					     cdesc->minor_version)) {
		dev_warn(ndev,
			 "Console nexus not initialized or invalid version\n");
		return -1;
	}

	used = !!(le32_to_cpu(cdesc->in_use) & (1 << console_num));
	if (in_use)
		*in_use = used;
	if (exclusive)
		*exclusive =
			!!(le32_to_cpu(cdesc->exclusive) & (1 << console_num));
	return !used;
}

/**
 * Extracts the platform data from the device tree
 *
 * @param	dev	serial device
 *
 * @return	0 for success, otherwise error
 */
/**
 * Initializes a PCIe console
 *
 * @param	dev	console device
 * @param	pcd	console descriptor
 * @param	console_num	console number
 *
 * NOTE: When this is called it is assumed that we already have exclusive
 * access to this console.
 */
static int octeontx_pcie_console_init(struct udevice *dev)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *cons;
	ofnode node = dev_ofnode(dev);
	fdt_addr_t addr;
	fdt_addr_t size;
	u32 input_buf_size, output_buf_size;
	int ret = 0;

	addr = ofnode_get_addr_size_index(node, 0, &size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err("%s: Could not read address and size\n", __func__);
		return -EINVAL;
	}
	input_buf_size = ofnode_read_u32_default(node, "rx-buffer-size", 0);
	output_buf_size = ofnode_read_u32_default(node, "tx-buffer-size", 0);
	debug("%s(%s) addr: %#llx, size: %#llx, input_buf_size: %#x, output_buf_size: %#x\n",
	      __func__, dev->name, addr, size, input_buf_size, output_buf_size);
	if (!input_buf_size) {
		dev_err(dev, "input-buf-size not defined for console %s\n",
			ofnode_get_name(node));
		return -EINVAL;
	}
	if (!output_buf_size) {
		dev_err(dev, "output-buf-size not defined for console %s\n",
			ofnode_get_name(node));
		return -ENOMEM;
	}
	if (size < sizeof(struct octeontx_pcie_console) +
			input_buf_size + output_buf_size) {
		dev_err(dev, "Not enough space reserved for buffers for %s, %#llx < %#lx + %#x + %#x\n",
			ofnode_get_name(node), size, sizeof(*cons),
			input_buf_size, output_buf_size);
		return -ENOMEM;
	}

	cons = (struct octeontx_pcie_console *)addr;
	if (cons->magic != OCTEONTX_PCIE_CONSOLE_MAGIC) {
		memset(cons, 0, sizeof(*cons));
		/*
		 * Note that the locks are not used yet but are planned
		 * in a future release update.
		 */
		octeontx_init_spin_lock(&cons->excl_lock);
		octeontx_pcie_init_target_lock(&cons->pcie_lock);
	}

	octeontx_pcie_target_lock(&cons->pcie_lock);
	snprintf(cons->name, sizeof(cons->name), dev->name);
	cons->owner_id = cpu_to_le32(OCTEONTX_PCIE_CONSOLE_OWNER_UBOOT);

	if (cons->host_console_connected) {
		/*
		 * If we're here then a host console is already connected
		 * so we can't change the pointers manipulated by the host.
		 */
		if (le32_to_cpu(cons->input_buf_size) != input_buf_size) {
			dev_err(dev,
				"host connected, input buffer size mismatch\n");
			ret = -EINVAL;
			goto error;
		}
		if (le32_to_cpu(cons->output_buf_size) != output_buf_size) {
			dev_err(dev,
				"host connected, output buffer size mismatch\n");
			ret = -EINVAL;
			goto error;
		}
		if (le64_to_cpu(cons->input_base_addr) +
		    le32_to_cpu(cons->input_buf_size) > size) {
			dev_err(dev, "Input buffer address invalid\n");
			ret = -EINVAL;
			goto error;
		}
		if (le64_to_cpu(cons->output_base_addr) +
		    le32_to_cpu(cons->output_buf_size) > size) {
			dev_err(dev, "Output buffer address invalid\n");
			ret = -EINVAL;
			goto error;
		}
		/* We should check for overlaps */
		/* Verify indices */
		/*
		 * We don't clear the indices so any data still in them can
		 * be read from input or printed from the output by the
		 * host client.
		 */
		if (le32_to_cpu(cons->input_write_index) >=
		    le32_to_cpu(cons->input_buf_size)) {
			dev_err(dev, "Input write index %u out of range\n",
				cons->input_write_index);
			ret = -EINVAL;
			goto error;
		}
		if (cons->input_read_index >= cons->input_buf_size) {
			dev_err(dev, "Input read index %u out of range\n",
				cons->input_read_index);
			ret = -EINVAL;
			goto error;
		}
		if (cons->output_write_index >= cons->output_buf_size) {
			dev_err(dev, "Output write index %u out of range\n",
				cons->output_write_index);
			ret = -EINVAL;
			goto error;
		}

		if (cons->output_read_index >= cons->output_buf_size) {
			dev_err(dev, "Output read index %u out of range\n",
				cons->output_read_index);
			ret = -EINVAL;
			goto error;
		}
	} else {
		debug("%s: Initializing console at %p\n", __func__, cons);
		/* Allocate input buffer immediately after the header */
		cons->input_base_addr = cpu_to_le64(addr + sizeof(*cons));
		cons->input_buf_size = cpu_to_le32(input_buf_size);
		/* Allocate the output buffer immediately after the input */
		cons->output_base_addr =
			cpu_to_le64(cons->input_base_addr + input_buf_size);
		cons->output_buf_size = cpu_to_le32(output_buf_size);

		cons->input_read_index = 0;
		cons->input_write_index = 0;
		cons->output_read_index = 0;
		cons->output_write_index = 0;
		cons->user = dev;
		debug("%s: input buffer: %#llx, %#x bytes, output buffer: %#llx, %#x bytes\n",
		      __func__, cons->input_base_addr, cons->input_buf_size,
		      cons->output_base_addr, cons->output_buf_size);
		__iowmb();
		cons->magic = OCTEONTX_PCIE_CONSOLE_MAGIC;
		__iowmb();
	}
error:
	octeontx_pcie_target_unlock(&cons->pcie_lock);

	if (!ret)
		priv->console = cons;

	return ret;
}

/**
 * Probe function for the PCIe console driver
 *
 * @param	dev	console device
 *
 * @return	0 for success, otherwise error
 */
static int octeontx_pcie_console_probe(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	struct udevice *parent = dev_get_parent(dev);
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console_nexus_priv *ppriv = dev_get_priv(parent);
	struct octeontx_pcie_console_nexus *nexus = ppriv->nexus;
	fdt_addr_t addr, size;
	struct stdio_dev sdev;
	int console_num;
	u64 old_mask, new_mask;
	bool ok;
	ulong start;
	int ret;
#ifdef DEBUG
	struct stdio_dev *_sdev;
#endif

	debug("%s(%s), parent: %p, ppriv: %p\n", __func__, dev->name,
	      parent, ppriv);
	if (!parent) {
		dev_err(dev, "Parent not available!\n");
		return -EINVAL;
	}
	if (ppriv->console) {
		debug("%s: console already bound\n", dev->name);
		return -ENODEV;
	}

	console_num = (int)dev_read_u32_default(dev, "reg", -1);
	if (console_num < 0) {
		dev_err(dev, "Invalid console number from reg\n");
		return -EINVAL;
	}
	debug("%s(%s): console number %d\n", __func__, dev->name, console_num);

	addr = ofnode_get_addr_size_index(node, 0, &size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Invalid address in device tree\n");
		return -EINVAL;
	}


	priv->nexus = ppriv->nexus;
	ret = ofnode_read_s32(node, "reg", &priv->console_num);
	if (ret) {
		dev_err(dev,
			"Could not read \"reg\" property from device tree!\n");
		return -EINVAL;
	}
	debug("%s(%s): console #%d\n", __func__, dev->name, priv->console_num);

	ret = octeontx_pcie_console_init(dev);
	if (ret) {
		dev_err(dev, "Error initializing console %s\n", dev->name);
		return ret;
	}

	if (ppriv->console) {
		debug("%s: Console already selected\n", __func__);
		return -ENODEV;
	}

	/* Set both in_use and exclusive */
	new_mask = 1ULL << console_num;
	new_mask |= (new_mask << 32);
	start = get_timer(0);
	do {
		if (nexus->in_use & (1 << console_num)) {
			debug("%s: console %d already in use\n",
			      __func__, console_num);
			return -ENODEV;
		}
		old_mask = nexus->in_use | (u64)(nexus->exclusive) << 32;
		debug("%s: in_use: %p, exclusive: %p\n", __func__,
		      &nexus->in_use, &nexus->exclusive);
		ok = __atomic_compare_exchange_n((u64 *)&nexus->in_use,
						 &old_mask, new_mask, false,
						 __ATOMIC_SEQ_CST,
						 __ATOMIC_SEQ_CST);
		debug("%s: ok: %d in_use: %#x, exclusive: %#x\n",
		      __func__, ok, nexus->in_use, nexus->exclusive);
	} while (!ok && get_timer(start) < 10);

	if (!ok) {
		dev_err(dev,
			"Atomic in_use failed, old_mask: %#llx, new_mask: %#llx\n",
				old_mask, new_mask);
		return -EIO;
	}

	memset(&sdev, 0, sizeof(sdev));

	sdev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT | DEV_FLAGS_DM;
	strncpy(sdev.name, "pci-console", sizeof(sdev.name));
	sdev.start = octeontx_pcie_console_stdio_start;
	sdev.stop = octeontx_pcie_console_stdio_stop;
	sdev.putc = octeontx_pcie_console_stdio_putc;
	sdev.puts = octeontx_pcie_console_stdio_puts;
	sdev.getc = octeontx_pcie_console_stdio_getc;
	sdev.tstc = octeontx_pcie_console_stdio_tstc;
	sdev.priv = dev;
	debug("%s: Registering stdio driver %s\n", __func__, sdev.name);
	ret = stdio_register_dev(&sdev, &priv->sdev);
#ifdef DEBUG
	_sdev = priv->sdev;
	debug("%s: stdio_register_dev returned %d, output: %p\n", __func__,
	      ret, _sdev);
	if (!ret) {
		struct list_head *pos;
		struct list_head *list = stdio_get_list();

		debug("%s: _sdev: %p, list: %p, next: %p, _sdev->next: %p, _sdev->prev: %p\n",
		      __func__, _sdev, list, list->next,
		      _sdev->list.next, _sdev->list.prev);
		list_for_each(pos, list) {
			debug("  pos: %p\n", pos);
			_sdev = list_entry(pos, struct stdio_dev, list);
			printf("stdio dev %s, %p, next: %p, prev: %p\n",
			       _sdev->name, _sdev, _sdev->list.next,
			       _sdev->list.prev);
		}
	}
#endif
	if (ret)
		dev_err(dev, "Error registering stdio device\n");
	else if (!ppriv->console)
		ppriv->console = priv->console;

	return ret;
}

int octeontx_pcie_console_ofdata_to_platdata(struct udevice *dev)
{
	struct octeontx_pcie_console_plat_data *plat = dev_get_platdata(dev);
	struct octeontx_pcie_console_plat_data *pplat;
	ofnode node = dev_ofnode(dev);
	fdt_addr_t addr;
	fdt_size_t size;
	int ret;

	addr = devfdt_get_addr_size_index(dev, 0, &size);
	debug("%s(%s): base: %#llx, size: %#llx\n", __func__, dev->name,
	      addr, size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Address and/or size not found in reg field\n");
		return -EINVAL;
	}
	pplat = dev_get_platdata(dev_get_parent(dev));
	if (!pplat) {
		dev_err(dev, "%s(%s): Error: parent platdata is NULL!\n",
			__func__, dev->name);
		return -EINVAL;
	}
	plat->nexus = pplat->nexus;
	plat->addr = ofnode_get_addr_size_index(node, 0, &plat->size);
	plat->base = (void *)plat->addr;
	plat->console = plat->base;
	ret = ofnode_read_resource(node, 0, &plat->res);
	debug("%s(%s): ret: %d\n", __func__, dev->name, ret);

	return ret;
}

static int modify_env(const char *name, const char *remove_name)
{
	char *env, *start, *end;
	char new_env[128];
	char temp[128];
	int len = strlen(remove_name);
	int dev;

	env = env_get(name);
	if (!env)
		return -1;

	strncpy(temp, env, sizeof(temp));
	new_env[0] = '\0';
	start = temp;
	do {
		end = strchr(start, ',');
		if (end)
			*end = '\0';

		if (strncmp(start, remove_name, len)) {
			if (start != temp)
				strncat(new_env, ",", sizeof(new_env));
			strncat(new_env, start, sizeof(new_env));
		}
		if (end)
			start = end + 1;
	} while (end && *start);
	if (!strcmp(name, "stdin"))
		dev = stdin;
	else if (!strcmp(name, "stdout"))
		dev = stdout;
	else if (!strcmp(name, "stderr"))
		dev = stderr;
	else
		return -ENODEV;

	return iomux_doenv(dev, new_env);
}

static int octeontx_pcie_console_remove(struct udevice *dev)
{
	struct octeontx_pcie_console_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console *cons = priv->console;
	struct octeontx_pcie_console_nexus *desc = priv->nexus;
	u64 mask = (1ULL << priv->console_num);

	dev_dbg(dev, "%s(%s): Performing cleanup\n", __func__, dev->name);

	/* We first need to remove ourselves from stdin, stdout and stderr */
	modify_env("stdin", DRIVER_NAME);
	modify_env("stdout", DRIVER_NAME);
	modify_env("stderr", DRIVER_NAME);

	stdio_deregister_dev(priv->sdev, 1);

	mask |= mask << 32;
	memset(cons->name, 0, sizeof(cons->name));
	if (cons->owner_id == OCTEONTX_PCIE_CONSOLE_OWNER_UBOOT)
		cons->owner_id = OCTEONTX_PCIE_CONSOLE_OWNER_UNUSED;

	/* Remove console from being in-use atomically */
	__atomic_fetch_and((u64 *)&desc->in_use, ~mask, __ATOMIC_SEQ_CST);

	dev_dbg(dev, "%s(%s): Performing cleanup done\n", __func__, dev->name);
	return 0;
}

int octeontx_pcie_console_init_nexus(struct udevice *dev)
{
	struct octeontx_pcie_console_plat_data *pdata = dev_get_platdata(dev);
	struct octeontx_pcie_console_nexus *pcd = pdata->nexus;
	ofnode node;
	fdt_addr_t addr;
	fdt_size_t size = 0;
	u64 new_hi, new_lo;
	u64 prev_hi, prev_lo;
	int num_consoles = 0;
	bool cmp;

	debug("%s(%s): Initializing main console descriptor\n",
	      __func__, dev->name);

#ifdef DEBUG
	debug("Start PCD\n");
	print_buffer(0, pcd, 1, sizeof(*pcd), 0);
#endif

	if (pcd->magic == cpu_to_le64(OCTEONTX_PCIE_CONSOLE_NEXUS_MAGIC)) {
		if (!octeontx_pcie_console_check_ver(pcd->major_version,
						     pcd->minor_version)) {
			dev_err(dev,
				"Error: console descriptor previously initialized to an unsupported version %u.%u\n",
				pcd->major_version, pcd->minor_version);
			return -EINVAL;
		}
		debug("%s: console descriptor already initialized\n",
		      dev->name);
		/* If already initialized then we're done. */
		if (pcd->num_consoles)
			return 0;
	}

	/*
	 * Fill in the number of consoles and their addresses.  This
	 * should be safe without requiring locks since if this is being
	 * initialized elsewhere the values will be identical.
	 */
	dev_for_each_subnode(node, dev) {
		if (num_consoles >= OCTEONTX_PCIE_MAX_CONSOLES) {
			dev_err(dev, "Too many PCIe consoles!  Max is %d\n",
				OCTEONTX_PCIE_MAX_CONSOLES);
			return -EINVAL;
		}
		addr = ofnode_get_addr_size_index(node, 0, &size);
		debug("%s: Address: %#llx, size: %#llx\n",
		      __func__, addr, size);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Could not get console %s address\n",
				ofnode_get_name(node));
			return -EINVAL;
		}
		debug("%s: %s addr: %#llx, size: %#llx\n", __func__,
		      ofnode_get_name(node), addr, size);
		if (!addr || !size) {
			dev_err(dev, "Console %s resources are 0!\n",
				ofnode_get_name(node));
			return -EINVAL;
		}
		/*
		 * It's safe to write to this even if previously initialized
		 * because the values are the same.
		 */
		pcd->console_addr[num_consoles++] = addr;
	}

	/*
	 * Make sure to flush the addresses before updating the number
	 * of consoles.
	 */
	__iowmb();
	/*
	 * It's safe to override this also since the value should be the same.
	 */
	if (num_consoles == 0) {
		dev_err(dev, "No consoles found in device tree!\n");
		return -ENODEV;
	}

	new_hi = cpu_to_le64(OCTEONTX_PCIE_CONSOLE_NEXUS_MAGIC);
	new_lo = ((u64)OCTEONTX_PCIE_CONSOLE_MAJOR << 0) |
		  (u64)OCTEONTX_PCIE_CONSOLE_MINOR << 8 |
		  (u64)0 /*i.e. flags */ << 16 |
		  (u64)num_consoles << 24;
	new_lo = cpu_to_le64(new_lo);

	debug("%s: Writing %#llx %#llx to %p\n", __func__, new_hi, new_lo, pcd);
	/* Now fill in the header atomically */
	cmp = octeontx_cmpxchg_atomic128(pcd, 0, 0, new_hi, new_lo,
					 &prev_hi, &prev_lo);
	/* If the old header is non-zero and the magic number matches... */
	if (!cmp && le64_to_cpu(prev_hi) == OCTEONTX_PCIE_CONSOLE_NEXUS_MAGIC) {
		prev_hi = le64_to_cpu(prev_hi);
		prev_lo = le64_to_cpu(prev_lo);
		debug("%s(%s): Pevious header: %#llx %#llx\n",
		      __func__, dev->name, prev_hi, prev_lo);
		if (!octeontx_pcie_console_check_ver(pcd->major_version,
						     pcd->minor_version)) {
			dev_err(dev,
				"Version mismatch during atomic operation, other version is %u.%u\n",
				pcd->major_version, pcd->minor_version);
			return -EINVAL;
		}
	}

#ifdef DEBUG
	debug("PCD after cmpxchg 128, cmp: %d\n", cmp);
	print_buffer(0, pcd, 1, sizeof(*pcd), 0);
#endif

	return 0;
}

static int octeontx_pcie_console_nexus_child_pre_probe(struct udevice *dev)
{
	debug("%s(%s)\n", __func__, dev->name);
	return 0;
}

static int octeontx_pcie_console_nexus_probe(struct udevice *dev)
{
	struct octeontx_pcie_console_nexus_priv *priv = dev_get_priv(dev);
	struct octeontx_pcie_console_plat_data *plat = dev_get_platdata(dev);
	struct octeontx_pcie_console_nexus *pcd;
	struct uclass *uc;
	struct udevice *sdev, *next;
	ofnode node = dev_ofnode(dev);
	int ret;
	static bool recursive_probe;

	debug("%s(%s)\n", __func__, dev->name);
	if (recursive_probe) {
		debug("%s(%s): recursive\n", __func__, dev->name);
		return 0;
	}

	debug("%s: dev: %p, priv: %p\n", __func__, dev, priv);
	pcd = (struct octeontx_pcie_console_nexus *)plat->nexus;
	if (!plat->nexus) {
		dev_err(dev, "Nexus pointer NULL!\n");
		return -ENODEV;
	}

	priv->console = NULL;
	priv->nexus = pcd;
	priv->console_node = node;
	ret = dev_read_resource(dev, 0, &priv->res);
	if (ret) {
		dev_err(dev, "Could not read resources\n");
		return -EINVAL;
	}

	ret = octeontx_pcie_console_init_nexus(dev);
	if (ret) {
		dev_err(dev,
			"Could not initialize PCIE console nexus descriptor\n");
		return -ENOENT;
	}

	ret = uclass_get(UCLASS_SERIAL, &uc);
	if (ret) {
		dev_err(dev, "%s: Could not get serial uclass\n", __func__);
		return ret;
	}
	device_foreach_child_safe(sdev, next, dev) {
		debug("%s: probing %s\n", __func__, sdev->name);
		ret = device_probe(sdev);
		if (!ret) {
			debug("%s(%s): Probed %s, done.\n",
			      __func__, dev->name, sdev->name);
			break;
		}
		if (ret != -ENODEV)
			dev_err(dev, "Error %d probing %s\n", ret, sdev->name);
	}
	return 0;
}

/**
 * Read device tree data for the platform data
 *
 * @param	dev	device to read
 *
 * @return	0 for success, -EINVAL if invalid address
 */
static int octeontx_pcie_console_nexus_ofdata_to_platdata(struct udevice *dev)
{
	struct octeontx_pcie_console_plat_data *plat = dev_get_platdata(dev);
	fdt_addr_t addr;
	fdt_size_t size;
	ofnode node = dev_ofnode(dev);

	if (!dev_of_valid(dev) || !dev_read_enabled(dev))
		return -ENOENT;


	addr = ofnode_get_addr_size_index(node, 0, &size);
	dev_dbg(dev, "%s(%s): base: 0x%llx, size: 0x%llx\n", __func__,
		dev->name, addr, size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Address and/or size not found in reg field\n");
		return -EINVAL;
	}
	if (size < sizeof(struct octeontx_pcie_console_nexus)) {
		dev_err(dev, "Nexus device tree size too small\n");
		return -EINVAL;
	}

	plat->size = size;
	plat->base = (void *)addr;
	plat->nexus = (struct octeontx_pcie_console_nexus *)addr;
	debug("%s(%s): address: %#llx, size: %#llx\n",
	      __func__, dev->name, addr, size);
	return 0;
}

static const struct udevice_id octeontx_pcie_console_nexus_serial_id[] = {
	{ .compatible = "marvell,pci-console-nexus", },
	{ },
};

U_BOOT_DRIVER(octeontx_pcie_console_nexus) = {
	.name = DRIVER_NAME "-nexus",
	.id = UCLASS_MISC,
	.flags = DM_FLAG_PRE_RELOC,
	.of_match = of_match_ptr(octeontx_pcie_console_nexus_serial_id),
	.ofdata_to_platdata = octeontx_pcie_console_nexus_ofdata_to_platdata,
	.platdata_auto_alloc_size =
				sizeof(struct octeontx_pcie_console_plat_data),
	.probe = octeontx_pcie_console_nexus_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_pcie_console_nexus_priv),
	.child_pre_probe = octeontx_pcie_console_nexus_child_pre_probe,
};

static const struct dm_serial_ops octeontx_pcie_console_ops = {
	.setbrg = octeontx_pcie_console_setbrg,
	.getc = octeontx_pcie_console_getc,
	.putc = octeontx_pcie_console_putc,
	.pending = octeontx_pcie_console_pending,
	.clear = octeontx_pcie_console_clear,
};

static const struct udevice_id octeontx_pcie_console_serial_id[] = {
	{ .compatible = "marvell,pci-console", },
	{ },
};

U_BOOT_DRIVER(octeontx_pcie_console) = {
	.name = DRIVER_NAME,
	.id = UCLASS_SERIAL,
	.ops = &octeontx_pcie_console_ops,
	.of_match = of_match_ptr(octeontx_pcie_console_serial_id),
	.probe = octeontx_pcie_console_probe,
	.ofdata_to_platdata = octeontx_pcie_console_ofdata_to_platdata,
	.remove = octeontx_pcie_console_remove,
	.priv_auto_alloc_size = sizeof(struct octeontx_pcie_console_priv),
	.platdata_auto_alloc_size =
				sizeof(struct octeontx_pcie_console_plat_data),
	.flags = DM_FLAG_OS_PREPARE | DM_FLAG_PRE_RELOC | DM_FLAG_ACTIVE_DMA,
};

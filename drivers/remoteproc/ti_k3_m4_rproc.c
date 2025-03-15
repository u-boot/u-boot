// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 M4 Remoteproc driver
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com/
 *	Hari Nagalla <hnagalla@ti.com>
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <remoteproc.h>
#include <errno.h>
#include <clk.h>
#include <reset.h>
#include <asm/io.h>
#include <power-domain.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include "ti_sci_proc.h"
#include <mach/security.h>

/**
 * struct k3_m4_mem - internal memory structure
 * @cpu_addr: MPU virtual address of the memory region
 * @bus_addr: Bus address used to access the memory region
 * @dev_addr: Device address from remoteproc view
 * @size: Size of the memory region
 */
struct k3_m4_mem {
	void __iomem *cpu_addr;
	phys_addr_t bus_addr;
	phys_addr_t dev_addr;
	size_t size;
};

/**
 * struct k3_m4_mem_data - memory definitions for m4 remote core
 * @name: name for this memory entry
 * @dev_addr: device address for the memory entry
 */
struct k3_m4_mem_data {
	const char *name;
	const u32 dev_addr;
};

/**
 * struct k3_m4_boot_data - internal data structure used for boot
 * @boot_align_addr: Boot vector address alignment granularity
 */
struct k3_m4_boot_data {
	u32 boot_align_addr;
};

/**
 * struct k3_m4_privdata - Structure representing Remote processor data.
 * @m4_rst:		m4 rproc reset control data
 * @tsp:		Pointer to TISCI proc contrl handle
 * @data:		Pointer to DSP specific boot data structure
 * @mem:		Array of available memories
 * @num_mem:		Number of available memories
 */
struct k3_m4_privdata {
	struct reset_ctl m4_rst;
	struct ti_sci_proc tsp;
	struct k3_m4_boot_data *data;
	struct k3_m4_mem *mem;
	int num_mems;
};

/*
 * The M4 cores have a local reset that affects only the CPU, and a
 * generic module reset that powers on the device and allows the M4 internal
 * memories to be accessed while the local reset is asserted. This function is
 * used to release the global reset on M4F to allow loading into the M4F
 * internal RAMs. This helper function is invoked in k3_m4_load() before any
 * actual firmware loading happens and is undone only in k3_m4_stop(). The local
 * reset cannot be released on M4 cores until after the firmware images are loaded.
 */
static int k3_m4_prepare(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);
	int ret;

	ret = ti_sci_proc_power_domain_on(&m4->tsp);
	if (ret)
		dev_err(dev, "cannot enable internal RAM loading, ret = %d\n",
			ret);

	return ret;
}

/*
 * This function is the counterpart to k3_m4_prepare() and is used to assert
 * the global reset on M4 cores. This completes the second step of powering
 * down the M4 cores. The cores themselves are halted through the local reset
 * in first step. This function is invoked in k3_m4_stop() after the local
 * reset is asserted.
 */
static int k3_m4_unprepare(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);

	return ti_sci_proc_power_domain_off(&m4->tsp);
}

/**
 * k3_m4_load() - Load up the Remote processor image
 * @dev:	rproc device pointer
 * @addr:	Address at which image is available
 * @size:	size of the image
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_m4_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);
	void *image_addr = (void *)addr;
	int ret;

	ret = ti_sci_proc_request(&m4->tsp);
	if (ret)
		return ret;

	ret = k3_m4_prepare(dev);
	if (ret) {
		dev_err(dev, "Prepare failed for core %d\n",
			m4->tsp.proc_id);
		goto proc_release;
	}

	ti_secure_image_post_process(&image_addr, &size);

	ret = rproc_elf_load_image(dev, addr, size);
	if (ret < 0) {
		dev_err(dev, "Loading elf failed %d\n", ret);
		goto unprepare;
	}

unprepare:
	if (ret)
		k3_m4_unprepare(dev);
proc_release:
	ti_sci_proc_release(&m4->tsp);
	return ret;
}

/**
 * k3_m4_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_m4_start(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);
	int ret;

	ret = ti_sci_proc_request(&m4->tsp);
	if (ret)
		return ret;

	ret = reset_deassert(&m4->m4_rst);

	ti_sci_proc_release(&m4->tsp);

	return ret;
}

static int k3_m4_stop(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);

	ti_sci_proc_request(&m4->tsp);
	reset_assert(&m4->m4_rst);
	k3_m4_unprepare(dev);
	ti_sci_proc_release(&m4->tsp);

	return 0;
}

static void *k3_m4_da_to_va(struct udevice *dev, ulong da, ulong len)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);
	phys_addr_t bus_addr, dev_addr;
	void __iomem *va = NULL;
	size_t size;
	u32 offset;
	int i;

	if (len <= 0)
		return NULL;

	for (i = 0; i < m4->num_mems; i++) {
		bus_addr = m4->mem[i].bus_addr;
		dev_addr = m4->mem[i].dev_addr;
		size = m4->mem[i].size;

		if (da >= dev_addr && ((da + len) <= (dev_addr + size))) {
			offset = da - dev_addr;
			va = m4->mem[i].cpu_addr + offset;
			return (__force void *)va;
		}

		if (da >= bus_addr && (da + len) <= (bus_addr + size)) {
			offset = da - bus_addr;
			va = m4->mem[i].cpu_addr + offset;
			return (__force void *)va;
		}
	}

	/* Assume it is DDR region and return da */
	return map_physmem(da, len, MAP_NOCACHE);
}

static const struct dm_rproc_ops k3_m4_ops = {
	.load = k3_m4_load,
	.start = k3_m4_start,
	.stop = k3_m4_stop,
	.device_to_virt = k3_m4_da_to_va,
};

static int ti_sci_proc_of_to_priv(struct udevice *dev, struct ti_sci_proc *tsp)
{
	u32 ids[2];
	int ret;

	tsp->sci = ti_sci_get_by_phandle(dev, "ti,sci");
	if (IS_ERR(tsp->sci)) {
		dev_err(dev, "ti_sci get failed: %ld\n", PTR_ERR(tsp->sci));
		return PTR_ERR(tsp->sci);
	}

	ret = dev_read_u32_array(dev, "ti,sci-proc-ids", ids, 2);
	if (ret) {
		dev_err(dev, "Proc IDs not populated %d\n", ret);
		return ret;
	}

	tsp->ops = &tsp->sci->ops.proc_ops;
	tsp->proc_id = ids[0];
	tsp->host_id = ids[1];
	tsp->dev_id = dev_read_u32_default(dev, "ti,sci-dev-id",
					   TI_SCI_RESOURCE_NULL);
	if (tsp->dev_id == TI_SCI_RESOURCE_NULL) {
		dev_err(dev, "Device ID not populated %d\n", ret);
		return -ENODEV;
	}

	return 0;
}

static const struct k3_m4_mem_data am6_m4_mems[] = {
	{ .name = "iram", .dev_addr = 0x0 },
	{ .name = "dram", .dev_addr = 0x30000 },
};

static int k3_m4_of_get_memories(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);
	int i;

	m4->num_mems = ARRAY_SIZE(am6_m4_mems);
	m4->mem = calloc(m4->num_mems, sizeof(*m4->mem));
	if (!m4->mem)
		return -ENOMEM;

	for (i = 0; i < m4->num_mems; i++) {
		m4->mem[i].bus_addr = dev_read_addr_size_name(dev,
							      am6_m4_mems[i].name,
					  (fdt_addr_t *)&m4->mem[i].size);
		if (m4->mem[i].bus_addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "%s bus address not found\n",
				am6_m4_mems[i].name);
			return -EINVAL;
		}
		m4->mem[i].cpu_addr = map_physmem(m4->mem[i].bus_addr,
						  m4->mem[i].size,
						  MAP_NOCACHE);
		m4->mem[i].dev_addr = am6_m4_mems[i].dev_addr;
	}

	return 0;
}

/**
 * k3_of_to_priv() - generate private data from device tree
 * @dev:	corresponding k3 m4 processor device
 * @m4:		pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_m4_of_to_priv(struct udevice *dev, struct k3_m4_privdata *m4)
{
	int ret;

	ret = reset_get_by_index(dev, 0, &m4->m4_rst);
	if (ret) {
		dev_err(dev, "reset_get() failed: %d\n", ret);
		return ret;
	}

	ret = ti_sci_proc_of_to_priv(dev, &m4->tsp);
	if (ret)
		return ret;

	ret =  k3_m4_of_get_memories(dev);
	if (ret)
		return ret;

	m4->data = (struct k3_m4_boot_data *)dev_get_driver_data(dev);

	return 0;
}

/**
 * k3_m4_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_m4_probe(struct udevice *dev)
{
	struct k3_m4_privdata *m4;
	int ret;

	m4 = dev_get_priv(dev);
	ret = k3_m4_of_to_priv(dev, m4);
	if (ret)
		return ret;

	/*
	 * The M4 local resets are deasserted by default on Power-On-Reset.
	 * Assert the local resets to ensure the M4s don't execute bogus code
	 * in .load() callback when the module reset is released to support
	 * internal memory loading. This is needed for M4 cores.
	 */
	reset_assert(&m4->m4_rst);

	return 0;
}

static int k3_m4_remove(struct udevice *dev)
{
	struct k3_m4_privdata *m4 = dev_get_priv(dev);

	free(m4->mem);

	return 0;
}

static const struct k3_m4_boot_data m4_data = {
	.boot_align_addr = SZ_1K,
};

static const struct udevice_id k3_m4_ids[] = {
	{ .compatible = "ti,am64-m4fss", .data = (ulong)&m4_data, },
	{}
};

U_BOOT_DRIVER(k3_m4) = {
	.name = "k3_m4",
	.of_match = k3_m4_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_m4_ops,
	.probe = k3_m4_probe,
	.remove = k3_m4_remove,
	.priv_auto = sizeof(struct k3_m4_privdata),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 HSM M4 Remoteproc driver
 *
 * Copyright (C) 2026 Texas Instruments Incorporated - http://www.ti.com/
 *	Beleswar Padhi <b-padhi@ti.com>
 */

#include <cpu_func.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <remoteproc.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include "ti_sci_proc.h"
#include <mach/security.h>
#include "../../arch/arm/mach-k3/common.h"

#define PROC_BOOT_CTRL_RESET_FLAG_HSM_M4 0x00000001

/**
 * struct k3_hsm_mem - internal memory structure
 * @cpu_addr: MPU virtual address of the memory region
 * @bus_addr: Bus address used to access the memory region
 * @dev_addr: Device address from remoteproc view
 * @size: Size of the memory region
 */
struct k3_hsm_mem {
	void __iomem *cpu_addr;
	phys_addr_t bus_addr;
	phys_addr_t dev_addr;
	size_t size;
};

/**
 * struct k3_hsm_mem_data - memory definitions for hsm remote core
 * @name: name for this memory entry
 * @dev_addr: device address for the memory entry
 */
struct k3_hsm_mem_data {
	const char *name;
	const u32 dev_addr;
};

/**
 * struct k3_hsm_privdata - Structure representing Remote processor data.
 * @tsp:		Pointer to TISCI proc control handle
 * @mem:		Array of available memories
 * @num_mems:		Number of available memories
 * @total_sram_size:	Total size of available memories
 */
struct k3_hsm_privdata {
	struct ti_sci_proc tsp;
	struct k3_hsm_mem *mem;
	int num_mems;
	size_t total_sram_size;
};

/**
 * k3_hsm_load() - Load up the Remote processor image
 * @dev:	rproc device pointer
 * @addr:	Address at which image is available
 * @size:	size of the image
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_hsm_load(struct udevice *dev, ulong addr, ulong size)
{
	struct k3_hsm_privdata *hsm = dev_get_priv(dev);
	void *image_addr = (void *)addr;
	size_t image_size = size;
	int ret;

	ret = ti_sci_proc_request(&hsm->tsp);
	if (ret)
		return ret;

	ret = ti_sci_proc_set_control(&hsm->tsp,
				      PROC_BOOT_CTRL_RESET_FLAG_HSM_M4, 0);
	if (ret)
		goto proc_release;

	ti_secure_image_check_binary(&image_addr, &image_size);
	ti_secure_image_post_process(&image_addr, &image_size);

	if (get_device_type() == K3_DEVICE_TYPE_GP) {
		/* Ensure firmware fits within total HSM SRAM bounds */
		if (size > hsm->total_sram_size) {
			dev_err(dev, "Firmware size (%lu) exceeds total SRAM size (%zu)\n",
				size, hsm->total_sram_size);
			ret = -EINVAL;
			goto proc_release;
		}

		debug("Loading HSM GP binary into SRAM0_0\n");
		memcpy((void *)hsm->mem[0].cpu_addr, image_addr, size);
		flush_dcache_range((unsigned long)hsm->mem[0].cpu_addr,
				   (unsigned long)(hsm->mem[0].cpu_addr + size));
	}

proc_release:
	ti_sci_proc_release(&hsm->tsp);
	return ret;
}

/**
 * k3_hsm_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int k3_hsm_start(struct udevice *dev)
{
	struct k3_hsm_privdata *hsm = dev_get_priv(dev);
	int ret;

	ret = ti_sci_proc_request(&hsm->tsp);
	if (ret)
		return ret;

	ret = ti_sci_proc_set_control(&hsm->tsp, 0,
				      PROC_BOOT_CTRL_RESET_FLAG_HSM_M4);

	ti_sci_proc_release(&hsm->tsp);

	return ret;
}

static int k3_hsm_stop(struct udevice *dev)
{
	struct k3_hsm_privdata *hsm = dev_get_priv(dev);
	int ret;

	ret = ti_sci_proc_request(&hsm->tsp);
	if (ret)
		return ret;

	ret = ti_sci_proc_set_control(&hsm->tsp,
				      PROC_BOOT_CTRL_RESET_FLAG_HSM_M4, 0);

	ti_sci_proc_release(&hsm->tsp);

	return ret;
}

static const struct dm_rproc_ops k3_hsm_ops = {
	.load = k3_hsm_load,
	.start = k3_hsm_start,
	.stop = k3_hsm_stop,
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
		dev_err(dev, "Device ID not populated\n");
		return -ENODEV;
	}

	return 0;
}

static const struct k3_hsm_mem_data hsm_mems[] = {
	{ .name = "sram0_0", .dev_addr = 0x0 },
	{ .name = "sram0_1", .dev_addr = 0x20000 },
	{ .name = "sram1",   .dev_addr = 0x30000 },
};

static int k3_hsm_of_get_memories(struct udevice *dev)
{
	struct k3_hsm_privdata *hsm = dev_get_priv(dev);
	int i;

	hsm->num_mems = ARRAY_SIZE(hsm_mems);
	hsm->mem = calloc(hsm->num_mems, sizeof(*hsm->mem));
	if (!hsm->mem)
		return -ENOMEM;

	for (i = 0; i < hsm->num_mems; i++) {
		hsm->mem[i].bus_addr = dev_read_addr_size_name(dev,
							       hsm_mems[i].name,
							       (fdt_addr_t *)&hsm->mem[i].size);
		if (hsm->mem[i].bus_addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "%s bus address not found\n",
				hsm_mems[i].name);
			return -EINVAL;
		}
		hsm->mem[i].cpu_addr = map_physmem(hsm->mem[i].bus_addr,
						   hsm->mem[i].size,
						   MAP_NOCACHE);
		hsm->mem[i].dev_addr = hsm_mems[i].dev_addr;
		hsm->total_sram_size += hsm->mem[i].size;
	}

	return 0;
}

/**
 * k3_hsm_probe() - Basic probe
 * @dev:	corresponding k3 remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int k3_hsm_probe(struct udevice *dev)
{
	struct k3_hsm_privdata *hsm;
	int ret;

	hsm = dev_get_priv(dev);

	ret = ti_sci_proc_of_to_priv(dev, &hsm->tsp);
	if (ret)
		return ret;

	ret = k3_hsm_of_get_memories(dev);
	if (ret)
		return ret;

	return 0;
}

static int k3_hsm_remove(struct udevice *dev)
{
	struct k3_hsm_privdata *hsm = dev_get_priv(dev);

	free(hsm->mem);

	return 0;
}

static const struct udevice_id k3_hsm_ids[] = {
	{ .compatible = "ti,hsm-m4fss" },
	{}
};

U_BOOT_DRIVER(k3_hsm) = {
	.name = "k3_hsm",
	.of_match = k3_hsm_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &k3_hsm_ops,
	.probe = k3_hsm_probe,
	.remove = k3_hsm_remove,
	.priv_auto = sizeof(struct k3_hsm_privdata),
};

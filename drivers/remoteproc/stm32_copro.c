// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#define pr_fmt(fmt) "%s: " fmt, __func__
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <regmap.h>
#include <remoteproc.h>
#include <reset.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/err.h>

#define RCC_GCR_HOLD_BOOT	0
#define RCC_GCR_RELEASE_BOOT	1

/**
 * struct stm32_copro_privdata - power processor private data
 * @reset_ctl:		reset controller handle
 * @hold_boot_regmap:	regmap for remote processor reset hold boot
 * @hold_boot_offset:	offset of the register controlling the hold boot setting
 * @hold_boot_mask:	bitmask of the register for the hold boot field
 * @rsc_table_addr:	resource table address
 */
struct stm32_copro_privdata {
	struct reset_ctl reset_ctl;
	struct regmap *hold_boot_regmap;
	uint hold_boot_offset;
	uint hold_boot_mask;
	ulong rsc_table_addr;
};

/**
 * stm32_copro_probe() - Basic probe
 * @dev:	corresponding STM32 remote processor device
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_probe(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	struct regmap *regmap;
	const fdt32_t *cell;
	int len, ret;

	priv = dev_get_priv(dev);

	regmap = syscon_regmap_lookup_by_phandle(dev, "st,syscfg-holdboot");
	if (IS_ERR(regmap)) {
		dev_err(dev, "unable to find holdboot regmap (%ld)\n",
			PTR_ERR(regmap));
		return PTR_ERR(regmap);
	}

	cell = dev_read_prop(dev, "st,syscfg-holdboot", &len);
	if (len < 3 * sizeof(fdt32_t)) {
		dev_err(dev, "holdboot offset and mask not available\n");
		return -EINVAL;
	}

	priv->hold_boot_regmap = regmap;
	priv->hold_boot_offset = fdtdec_get_number(cell + 1, 1);
	priv->hold_boot_mask = fdtdec_get_number(cell + 2, 1);

	ret = reset_get_by_index(dev, 0, &priv->reset_ctl);
	if (ret) {
		dev_err(dev, "failed to get reset (%d)\n", ret);
		return ret;
	}

	dev_dbg(dev, "probed\n");

	return 0;
}

/**
 * stm32_copro_set_hold_boot() - Hold boot bit management
 * @dev:	corresponding STM32 remote processor device
 * @hold:	hold boot value
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_set_hold_boot(struct udevice *dev, bool hold)
{
	struct stm32_copro_privdata *priv;
	uint val;
	int ret;

	priv = dev_get_priv(dev);

	val = hold ? RCC_GCR_HOLD_BOOT : RCC_GCR_RELEASE_BOOT;

	/*
	 * Note: shall run an SMC call (STM32_SMC_RCC) if platform is secured.
	 * To be updated when the code for this SMC service is available which
	 * is not the case for the time being.
	 */
	ret = regmap_update_bits(priv->hold_boot_regmap, priv->hold_boot_offset,
				 priv->hold_boot_mask, val);
	if (ret)
		dev_err(dev, "failed to set hold boot\n");

	return ret;
}

/**
 * stm32_copro_device_to_virt() - Convert device address to virtual address
 * @dev:	corresponding STM32 remote processor device
 * @da:		device address
 * @size:	Size of the memory region @da is pointing to
 * @return converted virtual address
 */
static void *stm32_copro_device_to_virt(struct udevice *dev, ulong da,
					ulong size)
{
	fdt32_t in_addr = cpu_to_be32(da), end_addr;
	u64 paddr;

	paddr = dev_translate_dma_address(dev, &in_addr);
	if (paddr == OF_BAD_ADDR) {
		dev_err(dev, "Unable to convert address %ld\n", da);
		return NULL;
	}

	end_addr = cpu_to_be32(da + size - 1);
	if (dev_translate_dma_address(dev, &end_addr) == OF_BAD_ADDR) {
		dev_err(dev, "Unable to convert address %ld\n", da + size - 1);
		return NULL;
	}

	return phys_to_virt(paddr);
}

/**
 * stm32_copro_load() - Loadup the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * @addr:	Address in memory where image is stored
 * @size:	Size in bytes of the image
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_load(struct udevice *dev, ulong addr, ulong size)
{
	struct stm32_copro_privdata *priv;
	ulong rsc_table_size;
	int ret;

	priv = dev_get_priv(dev);

	ret = stm32_copro_set_hold_boot(dev, true);
	if (ret)
		return ret;

	ret = reset_assert(&priv->reset_ctl);
	if (ret) {
		dev_err(dev, "Unable to assert reset line (ret=%d)\n", ret);
		return ret;
	}

	if (rproc_elf32_load_rsc_table(dev, addr, size, &priv->rsc_table_addr,
				       &rsc_table_size)) {
		priv->rsc_table_addr = 0;
		dev_warn(dev, "No valid resource table for this firmware\n");
	}

	return rproc_elf32_load_image(dev, addr, size);
}

/**
 * stm32_copro_start() - Start the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_start(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);

	/* move hold boot from true to false start the copro */
	ret = stm32_copro_set_hold_boot(dev, false);
	if (ret)
		return ret;

	/*
	 * Once copro running, reset hold boot flag to avoid copro
	 * rebooting autonomously
	 */
	ret = stm32_copro_set_hold_boot(dev, true);
	writel(ret ? TAMP_COPRO_STATE_OFF : TAMP_COPRO_STATE_CRUN,
	       TAMP_COPRO_STATE);
	if (!ret)
		/* Store rsc_address in bkp register */
		writel(priv->rsc_table_addr, TAMP_COPRO_RSC_TBL_ADDRESS);

	return ret;
}

/**
 * stm32_copro_reset() - Reset the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_reset(struct udevice *dev)
{
	struct stm32_copro_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);

	ret = stm32_copro_set_hold_boot(dev, true);
	if (ret)
		return ret;

	ret = reset_assert(&priv->reset_ctl);
	if (ret) {
		dev_err(dev, "Unable to assert reset line (ret=%d)\n", ret);
		return ret;
	}

	writel(TAMP_COPRO_STATE_OFF, TAMP_COPRO_STATE);

	return 0;
}

/**
 * stm32_copro_stop() - Stop the STM32 remote processor
 * @dev:	corresponding STM32 remote processor device
 * @return 0 if all went ok, else corresponding -ve error
 */
static int stm32_copro_stop(struct udevice *dev)
{
	return stm32_copro_reset(dev);
}

/**
 * stm32_copro_is_running() - Is the STM32 remote processor running
 * @dev:	corresponding STM32 remote processor device
 * @return 0 if the remote processor is running, 1 otherwise
 */
static int stm32_copro_is_running(struct udevice *dev)
{
	return (readl(TAMP_COPRO_STATE) == TAMP_COPRO_STATE_OFF);
}

static const struct dm_rproc_ops stm32_copro_ops = {
	.load = stm32_copro_load,
	.start = stm32_copro_start,
	.stop =  stm32_copro_stop,
	.reset = stm32_copro_reset,
	.is_running = stm32_copro_is_running,
	.device_to_virt = stm32_copro_device_to_virt,
};

static const struct udevice_id stm32_copro_ids[] = {
	{.compatible = "st,stm32mp1-m4"},
	{}
};

U_BOOT_DRIVER(stm32_copro) = {
	.name = "stm32_m4_proc",
	.of_match = stm32_copro_ids,
	.id = UCLASS_REMOTEPROC,
	.ops = &stm32_copro_ops,
	.probe = stm32_copro_probe,
	.priv_auto_alloc_size = sizeof(struct stm32_copro_privdata),
};

// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_NOP

#include <dm.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <linux/bitfield.h>
#include <mach/rif.h>

/* RIFSC offset register */
#define RIFSC_RISC_SECCFGR0(id)		(0x10 + 0x4 * (id))
#define RIFSC_RISC_PER0_CIDCFGR(id)	(0x100 + 0x8 * (id))
#define RIFSC_RISC_PER0_SEMCR(id)	(0x104 + 0x8 * (id))

/*
 * SEMCR register
 */
#define SEMCR_MUTEX			BIT(0)

/* RIFSC miscellaneous */
#define RIFSC_RISC_SCID_MASK		GENMASK(6, 4)
#define RIFSC_RISC_SEMWL_MASK		GENMASK(23, 16)

#define IDS_PER_RISC_SEC_PRIV_REGS	32

/*
 * CIDCFGR register fields
 */
#define CIDCFGR_CFEN			BIT(0)
#define CIDCFGR_SEMEN			BIT(1)

#define SEMWL_SHIFT			16

#define STM32MP25_RIFSC_ENTRIES		178

/* Compartiment IDs */
#define RIF_CID0			0x0
#define RIF_CID1			0x1

/*
 * struct stm32_rifsc_plat: Information about RIFSC device
 *
 * @base: Base address of RIFSC
 */
struct stm32_rifsc_plat {
	void *base;
};

/*
 * struct stm32_rifsc_child_plat: Information about each child
 *
 * @domain_id: Domain id
 */
struct stm32_rifsc_child_plat {
	u32 domain_id;
};

static bool stm32_rif_is_semaphore_available(void *base, u32 id)
{
	void *addr = base + RIFSC_RISC_PER0_SEMCR(id);

	return !(readl(addr) & SEMCR_MUTEX);
}

static int stm32_rif_acquire_semaphore(void *base, u32 id)
{
	void *addr = base + RIFSC_RISC_PER0_SEMCR(id);

	/* Check that the semaphore is available */
	if (!stm32_rif_is_semaphore_available(base, id))
		return -EACCES;

	setbits_le32(addr, SEMCR_MUTEX);

	/* Check that CID1 has the semaphore */
	if (stm32_rif_is_semaphore_available(base, id) ||
	    FIELD_GET(RIFSC_RISC_SCID_MASK, (readl(addr)) != RIF_CID1))
		return -EACCES;

	return 0;
}

static int stm32_rif_release_semaphore(void *base, u32 id)
{
	void *addr = base + RIFSC_RISC_PER0_SEMCR(id);

	if (stm32_rif_is_semaphore_available(base, id))
		return 0;

	clrbits_le32(addr, SEMCR_MUTEX);

	/* Ok if another compartment takes the semaphore before the check */
	if (!stm32_rif_is_semaphore_available(base, id) &&
	    FIELD_GET(RIFSC_RISC_SCID_MASK, (readl(addr)) == RIF_CID1))
		return -EACCES;

	return 0;
}

static int rifsc_parse_access_controller(ofnode node, struct ofnode_phandle_args *args)
{
	int ret;

	ret = ofnode_parse_phandle_with_args(node, "access-controllers",
					     "#access-controller-cells", 0,
					     0, args);
	if (ret) {
		log_debug("failed to parse access-controller (%d)\n", ret);
		return ret;
	}

	if (args->args_count != 1) {
		log_debug("invalid domain args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args[0] >= STM32MP25_RIFSC_ENTRIES) {
		log_err("Invalid sys bus ID for %s\n", ofnode_get_name(node));
		return -EINVAL;
	}

	return 0;
}

static int rifsc_check_access(void *base, u32 id)
{
	u32 reg_offset, reg_id, sec_reg_value, cid_reg_value, sem_reg_value;

	/*
	 * RIFSC_RISC_PRIVCFGRx and RIFSC_RISC_SECCFGRx both handle configuration access for
	 * 32 peripherals. On the other hand, there is one _RIFSC_RISC_PERx_CIDCFGR register
	 * per peripheral
	 */
	reg_id = id / IDS_PER_RISC_SEC_PRIV_REGS;
	reg_offset = id % IDS_PER_RISC_SEC_PRIV_REGS;
	sec_reg_value = readl(base + RIFSC_RISC_SECCFGR0(reg_id));
	cid_reg_value = readl(base + RIFSC_RISC_PER0_CIDCFGR(id));
	sem_reg_value = readl(base + RIFSC_RISC_PER0_SEMCR(id));

	/*
	 * First check conditions for semaphore mode, which doesn't take into
	 * account static CID.
	 */
	if (cid_reg_value & CIDCFGR_SEMEN)
		goto skip_cid_check;

	/*
	 * Skip cid check if CID filtering isn't enabled or filtering is enabled on CID0, which
	 * corresponds to whatever CID.
	 */
	if (!(cid_reg_value & CIDCFGR_CFEN) ||
	    FIELD_GET(RIFSC_RISC_SCID_MASK, cid_reg_value) == RIF_CID0)
		goto skip_cid_check;

	/* Coherency check with the CID configuration */
	if (FIELD_GET(RIFSC_RISC_SCID_MASK, cid_reg_value) != RIF_CID1) {
		log_debug("Invalid CID configuration for peripheral %d\n", id);
		return -EACCES;
	}

	/* Check semaphore accesses */
	if (cid_reg_value & CIDCFGR_SEMEN) {
		if (!(FIELD_GET(RIFSC_RISC_SEMWL_MASK, cid_reg_value) & BIT(RIF_CID1))) {
			log_debug("Not in semaphore whitelist for peripheral %d\n", id);
			return -EACCES;
		}
		if (!stm32_rif_is_semaphore_available(base, id) &&
		    !(FIELD_GET(RIFSC_RISC_SCID_MASK, sem_reg_value) & BIT(RIF_CID1))) {
			log_debug("Semaphore unavailable for peripheral %d\n", id);
			return -EACCES;
		}
	}

skip_cid_check:
	/* Check security configuration */
	if (sec_reg_value & BIT(reg_offset)) {
		log_debug("Invalid security configuration for peripheral %d\n", id);
		return -EACCES;
	}

	return 0;
}

int stm32_rifsc_check_access_by_id(ofnode device_node, u32 id)
{
	struct ofnode_phandle_args args;
	int err;

	if (id >= STM32MP25_RIFSC_ENTRIES)
		return -EINVAL;

	err = rifsc_parse_access_controller(device_node, &args);
	if (err)
		return err;

	return rifsc_check_access((void *)ofnode_get_addr(args.node), id);
}

int stm32_rifsc_check_access(ofnode device_node)
{
	struct ofnode_phandle_args args;
	int err;

	err = rifsc_parse_access_controller(device_node, &args);
	if (err)
		return err;

	return rifsc_check_access((void *)ofnode_get_addr(args.node), args.args[0]);
}

static int stm32_rifsc_child_pre_probe(struct udevice *dev)
{
	struct stm32_rifsc_plat *plat = dev_get_plat(dev->parent);
	struct stm32_rifsc_child_plat *child_plat = dev_get_parent_plat(dev);
	u32 cid_reg_value;
	int err;
	u32 id = child_plat->domain_id;

	cid_reg_value = readl(plat->base + RIFSC_RISC_PER0_CIDCFGR(id));

	/*
	 * If the peripheral is in semaphore mode, take the semaphore so that
	 * the CID1 has the ownership.
	 */
	if (cid_reg_value & CIDCFGR_SEMEN &&
	    (FIELD_GET(RIFSC_RISC_SEMWL_MASK, cid_reg_value) & BIT(RIF_CID1))) {
		err = stm32_rif_acquire_semaphore(plat->base, id);
		if (err) {
			dev_err(dev, "Couldn't acquire RIF semaphore for peripheral %d (%d)\n",
				id, err);
			return err;
		}
		dev_dbg(dev, "Acquiring semaphore for peripheral %d\n", id);
	}

	return 0;
}

static int stm32_rifsc_child_post_remove(struct udevice *dev)
{
	struct stm32_rifsc_plat *plat = dev_get_plat(dev->parent);
	struct stm32_rifsc_child_plat *child_plat = dev_get_parent_plat(dev);
	u32 cid_reg_value;
	int err;
	u32 id = child_plat->domain_id;

	cid_reg_value = readl(plat->base + RIFSC_RISC_PER0_CIDCFGR(id));

	/*
	 * If the peripheral is in semaphore mode, release the semaphore so that
	 * there's no ownership.
	 */
	if (cid_reg_value & CIDCFGR_SEMEN &&
	    (FIELD_GET(RIFSC_RISC_SEMWL_MASK, cid_reg_value) & BIT(RIF_CID1))) {
		err = stm32_rif_release_semaphore(plat->base, id);
		if (err)
			dev_err(dev, "Couldn't release rif semaphore for peripheral %d (%d)\n",
				id, err);
	}

	return 0;
}

static int stm32_rifsc_child_post_bind(struct udevice *dev)
{
	struct stm32_rifsc_child_plat *child_plat = dev_get_parent_plat(dev);
	struct ofnode_phandle_args args;
	int ret;

	if (!dev_has_ofnode(dev))
		return -EPERM;

	ret = rifsc_parse_access_controller(dev_ofnode(dev), &args);
	if (ret)
		return ret;

	child_plat->domain_id = args.args[0];

	return 0;
}

static int stm32_rifsc_bind(struct udevice *dev)
{
	struct stm32_rifsc_plat *plat = dev_get_plat(dev);
	struct ofnode_phandle_args args;
	int ret = 0, err = 0;
	ofnode node;

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base) {
		dev_err(dev, "can't get registers base address\n");
		return -ENOENT;
	}

	for (node = ofnode_first_subnode(dev_ofnode(dev));
	     ofnode_valid(node);
	     node = ofnode_next_subnode(node)) {
		const char *node_name = ofnode_get_name(node);

		if (!ofnode_is_enabled(node))
			continue;

		err = rifsc_parse_access_controller(node, &args);
		if (err) {
			dev_dbg(dev, "%s failed to parse child on bus (%d)\n", node_name, err);
			continue;
		}

		err = rifsc_check_access(plat->base, args.args[0]);
		if (err) {
			dev_info(dev, "%s not allowed on bus (%d)\n", node_name, err);
			continue;
		}

		err = lists_bind_fdt(dev, node, NULL, NULL,
				     gd->flags & GD_FLG_RELOC ? false : true);
		if (err && !ret) {
			ret = err;
			dev_err(dev, "%s failed to bind on bus (%d)\n", node_name, ret);
		}
	}

	if (ret)
		dev_err(dev, "Some child failed to bind (%d)\n", ret);

	return ret;
}

static int stm32_rifsc_remove(struct udevice *bus)
{
	struct udevice *dev;

	/* Deactivate all child devices not yet removed */
	for (device_find_first_child(bus, &dev); dev; device_find_next_child(&dev))
		if (device_active(dev))
			stm32_rifsc_child_post_remove(dev);

	return 0;
}

static const struct udevice_id stm32_rifsc_ids[] = {
	{ .compatible = "st,stm32mp25-rifsc" },
	{},
};

U_BOOT_DRIVER(stm32_rifsc) = {
	.name = "stm32_rifsc",
	.id = UCLASS_NOP,
	.of_match = stm32_rifsc_ids,
	.bind = stm32_rifsc_bind,
	.remove = stm32_rifsc_remove,
	.child_post_bind = stm32_rifsc_child_post_bind,
	.child_pre_probe = stm32_rifsc_child_pre_probe,
	.child_post_remove = stm32_rifsc_child_post_remove,
	.plat_auto = sizeof(struct stm32_rifsc_plat),
	.per_child_plat_auto = sizeof(struct stm32_rifsc_child_plat),
	.flags = DM_FLAG_OS_PREPARE,
};

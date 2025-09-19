// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_NOP

#include <clk.h>
#include <dm.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_addr.h>
#include <dm/of_access.h>
#include <linux/bitfield.h>
#include <linux/ioport.h>
#include <mach/rif.h>

/* OCTOSPI control register */
#define OCTOSPIM_CR		0
#define CR_MUXEN		BIT(0)
#define CR_MUXENMODE_MASK	GENMASK(1, 0)
#define CR_CSSEL_OVR_EN		BIT(4)
#define CR_CSSEL_OVR_MASK	GENMASK(6, 5)
#define CR_REQ2ACK_MASK		GENMASK(23, 16)

#define OMM_CHILD_NB		2
#define OMM_CLK_NB		3
#define OMM_RESET_NB		3
#define NSEC_PER_SEC		1000000000L

struct stm32_omm_plat {
	phys_addr_t regs_base;
	struct regmap *syscfg_regmap;
	struct clk clk[OMM_CLK_NB];
	struct reset_ctl reset_ctl[OMM_RESET_NB];
	resource_size_t mm_ospi2_size;
	u32 mux;
	u32 cssel_ovr;
	u32 req2ack;
	u32 amcr_base;
	u32 amcr_mask;
	unsigned long clk_rate_max;
	u8 nb_child;
};

static int stm32_omm_set_amcr(struct udevice *dev, bool set)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	unsigned int amcr, read_amcr;

	amcr = plat->mm_ospi2_size / SZ_64M;

	if (set)
		regmap_update_bits(plat->syscfg_regmap, plat->amcr_base,
				   plat->amcr_mask, amcr);

	/* read AMCR and check coherency with memory-map areas defined in DT */
	regmap_read(plat->syscfg_regmap, plat->amcr_base, &read_amcr);
	read_amcr = read_amcr >> (ffs(plat->amcr_mask) - 1);

	if (amcr != read_amcr) {
		dev_err(dev, "AMCR value not coherent with DT memory-map areas\n");
		return -EINVAL;
	}

	return 0;
}

static int stm32_omm_toggle_child_clock(struct udevice *dev, bool enable)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	int i, ret;

	for (i = 0; i < plat->nb_child; i++) {
		if (enable) {
			ret = clk_enable(&plat->clk[i + 1]);
			if (ret) {
				dev_err(dev, "Can not enable clock\n");
				goto clk_error;
			}
		} else {
			clk_disable(&plat->clk[i + 1]);
		}
	}

	return 0;

clk_error:
	while (i--)
		clk_disable(&plat->clk[i + 1]);

	return ret;
}

static int stm32_omm_disable_child(struct udevice *dev)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	int ret;
	u8 i;

	ret = stm32_omm_toggle_child_clock(dev, true);
	if (ret)
		return ret;

	for (i = 0; i < plat->nb_child; i++) {
		/* reset OSPI to ensure CR_EN bit is set to 0 */
		reset_assert(&plat->reset_ctl[i + 1]);
		udelay(2);
		reset_deassert(&plat->reset_ctl[i + 1]);
	}

	return stm32_omm_toggle_child_clock(dev, false);
}

static int stm32_omm_configure(struct udevice *dev)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	int ret;
	u32 mux = 0;
	u32 cssel_ovr = 0;
	u32 req2ack = 0;

	/* Ensure both OSPI instance are disabled before configuring OMM */
	ret = stm32_omm_disable_child(dev);
	if (ret)
		return ret;

	ret = clk_enable(&plat->clk[0]);
	if (ret) {
		dev_err(dev, "Failed to enable OMM clock (%d)\n", ret);
		return ret;
	}

	reset_assert(&plat->reset_ctl[0]);
	udelay(2);
	reset_deassert(&plat->reset_ctl[0]);

	if (plat->mux & CR_MUXEN) {
		if (plat->req2ack) {
			req2ack = DIV_ROUND_UP(plat->req2ack,
					       NSEC_PER_SEC / plat->clk_rate_max) - 1;
			if (req2ack > 256)
				req2ack = 256;
		}

		req2ack = FIELD_PREP(CR_REQ2ACK_MASK, req2ack);
		clrsetbits_le32(plat->regs_base + OCTOSPIM_CR, CR_REQ2ACK_MASK,
				req2ack);

		/*
		 * If the mux is enabled, the 2 OSPI clocks have to be
		 * always enabled
		 */
		ret = stm32_omm_toggle_child_clock(dev, true);
		if (ret)
			return ret;
	}

	if (plat->cssel_ovr != 0xff) {
		cssel_ovr = FIELD_PREP(CR_CSSEL_OVR_MASK, cssel_ovr);
		cssel_ovr |= CR_CSSEL_OVR_EN;
		clrsetbits_le32(plat->regs_base + OCTOSPIM_CR, CR_CSSEL_OVR_MASK,
				cssel_ovr);
	}

	mux = FIELD_PREP(CR_MUXENMODE_MASK, plat->mux);
	clrsetbits_le32(plat->regs_base + OCTOSPIM_CR, CR_MUXENMODE_MASK, mux);
	clk_disable(&plat->clk[0]);

	return stm32_omm_set_amcr(dev, true);
}

static void stm32_omm_release_childs(ofnode *child_list, u8 nb_child)
{
	u8 i;

	for (i = 0; i < nb_child; i++)
		stm32_rifsc_release_access(child_list[i]);
}

static int stm32_omm_probe(struct udevice *dev)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	ofnode child_list[OMM_CHILD_NB];
	ofnode child;
	int ret;
	u8 child_access_granted = 0;
	bool child_access[OMM_CHILD_NB];

	/* check child's access */
	for (child = ofnode_first_subnode(dev_ofnode(dev));
	     ofnode_valid(child);
	     child = ofnode_next_subnode(child)) {
		if (plat->nb_child > OMM_CHILD_NB) {
			dev_err(dev, "Bad DT, found too much children\n");
			return -E2BIG;
		}

		if (!ofnode_device_is_compatible(child, "st,stm32mp25-ospi"))
			return -EINVAL;

		ret = stm32_rifsc_grant_access(child);
		if (ret < 0 && ret != -EACCES)
			return ret;

		child_access[plat->nb_child] = false;
		if (!ret) {
			child_access_granted++;
			child_access[plat->nb_child] = true;
		}

		child_list[plat->nb_child] = child;
		plat->nb_child++;
	}

	if (plat->nb_child != OMM_CHILD_NB)
		return -EINVAL;

	/* check if OMM's resource access is granted */
	ret = stm32_rifsc_grant_access(dev_ofnode(dev));
	if (ret < 0 && ret != -EACCES)
		goto end;

	/* All child's access are granted ? */
	if (!ret && child_access_granted == plat->nb_child) {
		ret = stm32_omm_configure(dev);
		if (ret)
			goto end;
	} else {
		dev_dbg(dev, "Octo Memory Manager resource's access not granted\n");
		/*
		 * AMCR can't be set, so check if current value is coherent
		 * with memory-map areas defined in DT
		 */
		ret = stm32_omm_set_amcr(dev, false);
	}

end:
	stm32_omm_release_childs(child_list, plat->nb_child);
	stm32_rifsc_release_access(dev_ofnode(dev));

	return ret;
}

static int stm32_omm_of_to_plat(struct udevice *dev)
{
	struct stm32_omm_plat *plat = dev_get_plat(dev);
	static const char * const clocks_name[] = {"omm", "ospi1", "ospi2"};
	static const char * const mm_name[] = { "ospi1", "ospi2" };
	static const char * const resets_name[] = {"omm", "ospi1", "ospi2"};
	struct resource res, res1, mm_res;
	struct ofnode_phandle_args args;
	struct udevice *child;
	unsigned long clk_rate;
	struct clk child_clk;
	int ret, idx;
	u8 i;

	plat->regs_base = dev_read_addr(dev);
	if (plat->regs_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_resource_byname(dev, "memory_map", &mm_res);
	if (ret) {
		dev_err(dev, "can't get omm_mm mmap resource(ret = %d)!\n", ret);
		return ret;
	}

	for (i = 0; i < OMM_CLK_NB; i++) {
		ret = clk_get_by_name(dev, clocks_name[i], &plat->clk[i]);
		if (ret < 0) {
			dev_err(dev, "Can't find I/O manager clock %s\n", clocks_name[i]);
			return ret;
		}

		ret = reset_get_by_name(dev, resets_name[i], &plat->reset_ctl[i]);
		if (ret < 0) {
			dev_err(dev, "Can't find I/O manager reset %s\n", resets_name[i]);
			return ret;
		}
	}

	/* parse children's clock */
	plat->clk_rate_max = 0;
	device_foreach_child(child, dev) {
		ret = clk_get_by_index(child, 0, &child_clk);
		if (ret) {
			dev_err(dev, "Failed to get clock for %s\n",
				dev_read_name(child));
			return ret;
		}

		clk_rate = clk_get_rate(&child_clk);
		if (!clk_rate) {
			dev_err(dev, "Invalid clock rate\n");
			return -EINVAL;
		}

		if (clk_rate > plat->clk_rate_max)
			plat->clk_rate_max = clk_rate;
	}

	plat->mux = dev_read_u32_default(dev, "st,omm-mux", 0);
	plat->req2ack = dev_read_u32_default(dev, "st,omm-req2ack-ns", 0);
	plat->cssel_ovr = dev_read_u32_default(dev, "st,omm-cssel-ovr", 0xff);
	plat->mm_ospi2_size = 0;

	for (i = 0; i < 2; i++) {
		idx = dev_read_stringlist_search(dev, "memory-region-names",
						 mm_name[i]);
		if (idx < 0)
			continue;

		/* res1 only used on second loop iteration */
		res1.start = res.start;
		res1.end = res.end;

		dev_read_phandle_with_args(dev, "memory-region", NULL, 0, idx,
					   &args);
		ret = ofnode_read_resource(args.node, 0, &res);
		if (ret) {
			dev_err(dev, "unable to resolve memory region\n");
			return ret;
		}

		/* check that memory region fits inside OMM memory map area */
		if (!resource_contains(&mm_res, &res)) {
			dev_err(dev, "%s doesn't fit inside OMM memory map area\n",
				mm_name[i]);
			dev_err(dev, "[0x%llx-0x%llx] doesn't fit inside [0x%llx-0x%llx]\n",
				res.start, res.end,
				mm_res.start, mm_res.end);

			return -EFAULT;
		}

		if (i == 1) {
			plat->mm_ospi2_size = resource_size(&res);

			/* check that OMM memory region 1 doesn't overlap memory region 2 */
			if (resource_overlaps(&res, &res1)) {
				dev_err(dev, "OMM memory-region %s overlaps memory region %s\n",
					mm_name[0], mm_name[1]);
				dev_err(dev, "[0x%llx-0x%llx] overlaps [0x%llx-0x%llx]\n",
					res1.start, res1.end, res.start, res.end);

				return -EFAULT;
			}
		}
	}

	plat->syscfg_regmap = syscon_regmap_lookup_by_phandle(dev, "st,syscfg-amcr");
	if (IS_ERR(plat->syscfg_regmap)) {
		dev_err(dev, "Failed to get st,syscfg-amcr property\n");
		ret = PTR_ERR(plat->syscfg_regmap);
		return ret;
	}

	ret = dev_read_u32_index(dev, "st,syscfg-amcr", 1, &plat->amcr_base);
	if (ret) {
		dev_err(dev, "Failed to get st,syscfg-amcr base\n");
		return ret;
	}

	ret = dev_read_u32_index(dev, "st,syscfg-amcr", 2, &plat->amcr_mask);
	if (ret) {
		dev_err(dev, "Failed to get st,syscfg-amcr mask\n");
		return ret;
	}

	return 0;
};

static int stm32_omm_bind(struct udevice *dev)
{
	int ret = 0, err = 0;
	ofnode node;

	for (node = ofnode_first_subnode(dev_ofnode(dev));
	     ofnode_valid(node);
	     node = ofnode_next_subnode(node)) {
		const char *node_name = ofnode_get_name(node);

		if (!ofnode_is_enabled(node) || stm32_rifsc_grant_access(node)) {
			dev_dbg(dev, "%s failed to bind\n", node_name);
			continue;
		}

		err = lists_bind_fdt(dev, node, NULL, NULL,
				     gd->flags & GD_FLG_RELOC ? false : true);
		if (err && !ret) {
			ret = err;
			dev_dbg(dev, "%s: ret=%d\n", node_name, ret);
		}
	}

	if (ret)
		dev_dbg(dev, "Some drivers failed to bind\n");

	return ret;
}

static const struct udevice_id stm32_omm_ids[] = {
	{ .compatible = "st,stm32mp25-omm", },
	{},
};

U_BOOT_DRIVER(stm32_omm) = {
	.name		= "stm32_omm",
	.id		= UCLASS_NOP,
	.probe		= stm32_omm_probe,
	.of_match	= stm32_omm_ids,
	.of_to_plat	= stm32_omm_of_to_plat,
	.plat_auto	= sizeof(struct stm32_omm_plat),
	.bind		= stm32_omm_bind,
};

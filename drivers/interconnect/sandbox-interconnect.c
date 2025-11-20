// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025 Linaro Limited
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <interconnect-uclass.h>
#include <asm/io.h>
#include <interconnect.h>
#include <linux/err.h>

#define MAX_LINKS	2

struct sandbox_interconnect_node {
	const char *name;
	unsigned int num_links;
	struct sandbox_interconnect_node *links[MAX_LINKS];
	u64 avg_bw;
	u64 peak_bw;
};

struct sandbox_interconnect_data {
	struct sandbox_interconnect_node **nodes;
	const unsigned int num_nodes;
};

struct sandbox_interconnect_provider {
	struct udevice *dev;
	struct sandbox_interconnect_data *data;
	u64 avg;
	u64 peak;
};

/*
 * Node graph:
 *                   ______________________________
 *  [ NODE0 ]--\    /                              \   /-->[ NODE3 ]
 *              |-->| NODE2_SLAVE --> NODE2_MASTER |--|
 *  [ NODE1 ]--/    \______________________________/   \-->[ NODE4 ]
 *
 */

static struct sandbox_interconnect_node node2_slave;
static struct sandbox_interconnect_node node2_master;
static struct sandbox_interconnect_node node3;
static struct sandbox_interconnect_node node4;

static struct sandbox_interconnect_node node0 = {
	.name = "node0",
	.num_links = 1,
	.links = { &node2_slave },
};

static struct sandbox_interconnect_node node1 = {
	.name = "node1",
	.num_links = 1,
	.links = { &node2_slave },
};

static struct sandbox_interconnect_node node2_slave = {
	.name = "node2_slave",
	.num_links = 1,
	.links = { &node2_master },
};

static struct sandbox_interconnect_node node2_master = {
	.name = "node2_master",
	.num_links = 2,
	.links = { &node3, &node4 },
};

static struct sandbox_interconnect_node node3 = {
	.name = "node3",
};

static struct sandbox_interconnect_node node4 = {
	.name = "node4",
};

/* xlate mapping */
static struct sandbox_interconnect_node *interconnect0_nodes[] = {
	[0] = &node0,
};

static struct sandbox_interconnect_node *interconnect1_nodes[] = {
	[0] = &node1,
};

static struct sandbox_interconnect_node *interconnect2_nodes[] = {
	[0] = &node2_slave,
	[1] = &node2_master,
};

static struct sandbox_interconnect_node *interconnect3_nodes[] = {
	[0] = &node3,
};

static struct sandbox_interconnect_node *interconnect4_nodes[] = {
	[0] = &node4,
};

static struct sandbox_interconnect_data interconnect0_data = {
	.nodes = interconnect0_nodes,
	.num_nodes = ARRAY_SIZE(interconnect0_nodes),
};

static struct sandbox_interconnect_data interconnect1_data = {
	.nodes = interconnect1_nodes,
	.num_nodes = ARRAY_SIZE(interconnect1_nodes),
};

static struct sandbox_interconnect_data interconnect2_data = {
	.nodes = interconnect2_nodes,
	.num_nodes = ARRAY_SIZE(interconnect2_nodes),
};

static struct sandbox_interconnect_data interconnect3_data = {
	.nodes = interconnect3_nodes,
	.num_nodes = ARRAY_SIZE(interconnect3_nodes),
};

static struct sandbox_interconnect_data interconnect4_data = {
	.nodes = interconnect4_nodes,
	.num_nodes = ARRAY_SIZE(interconnect4_nodes),
};

int sandbox_interconnect_get_bw(struct udevice *dev, u64 *avg, u64 *peak)
{
	struct sandbox_interconnect_provider *priv = dev_get_plat(dev);

	*avg = priv->avg;
	*peak = priv->peak;

	return 0;
}

static int sandbox_interconnect_links_aggregate(struct udevice *dev)
{
	struct sandbox_interconnect_provider *priv = dev_get_plat(dev);
	u64 avg = 0, peak = 0;
	int i;

	debug("(provider=%s)\n", dev->name);

	for (i = 0; i < priv->data->num_nodes; i++) {
		struct sandbox_interconnect_node *sandbox_node = priv->data->nodes[i];

		if (!sandbox_node)
			continue;

		avg += sandbox_node->avg_bw;
		peak = max_t(u32, sandbox_node->peak_bw, peak);
	}

	priv->avg = avg / priv->data->num_nodes;
	priv->peak = peak;

	debug("(provider=%s,avg=%llu peak=%llu)\n",
	      dev->name, priv->avg, priv->peak);

	return 0;
}

static int sandbox_interconnect_set(struct icc_node *src, struct icc_node *dst)
{
	struct icc_node *node;

	debug("(src=%s,dst=%s)\n", src->dev->name, dst->dev->name);

	if (!src)
		node = dst;
	else
		node = src;

	return sandbox_interconnect_links_aggregate(node->dev->parent);
}

static int sandbox_interconnect_aggregate(struct icc_node *node, u32 tag, u32 avg_bw,
					  u32 peak_bw, u32 *agg_avg, u32 *agg_peak)
{
	struct sandbox_interconnect_node *sandbox_node = node->data;

	debug("(node=%s,tag=%d,avg=%u,peak=%u)\n",
	      node->dev->name, tag, avg_bw, peak_bw);

	sandbox_node->avg_bw += avg_bw;
	sandbox_node->peak_bw = max_t(u32, sandbox_node->peak_bw, peak_bw);

	*agg_avg += avg_bw;
	*agg_peak = max_t(u32, *agg_peak, peak_bw);

	debug("(node=%s,new avg=%llu,new peak=%llu)\n",
	      node->dev->name, sandbox_node->avg_bw, sandbox_node->peak_bw);

	return 0;
}

static void sandbox_interconnect_pre_aggregate(struct icc_node *node)
{
	struct sandbox_interconnect_node *sandbox_node = node->data;

	debug("(node=%s)\n", node->dev->name);

	sandbox_node->avg_bw = 0;
	sandbox_node->peak_bw = 0;
}

static struct icc_node *sandbox_interconnect_xlate(struct udevice *dev,
						   const struct ofnode_phandle_args *spec)
{
	struct icc_provider *plat = dev_get_uclass_plat(dev);
	unsigned int idx = spec->args[0];

	debug("(dev=%s)\n", dev->name);

	if (idx >= plat->xlate_num_nodes) {
		pr_err("%s: invalid index %u\n", __func__, idx);
		return ERR_PTR(-EINVAL);
	}

	return plat->xlate_nodes[idx];
}

static int sandbox_interconnect_bind(struct udevice *dev)
{
	struct sandbox_interconnect_provider *priv = dev_get_plat(dev);
	struct icc_provider *plat = dev_get_uclass_plat(dev);
	size_t i;

	debug("(dev=%s)\n", dev->name);

	priv->data = (struct sandbox_interconnect_data *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	plat->xlate_num_nodes = priv->data->num_nodes;
	plat->xlate_nodes = calloc(sizeof(struct icc_node *), priv->data->num_nodes);
	if (!plat->xlate_nodes)
		return -ENOMEM;

	priv->dev = dev;

	for (i = 0; i < priv->data->num_nodes; i++) {
		struct sandbox_interconnect_node *sandbox_node;
		struct icc_node *node;
		int j;

		sandbox_node = priv->data->nodes[i];
		if (!sandbox_node)
			continue;

		node = icc_node_create(dev, (ulong)sandbox_node,
				       sandbox_node->name);
		if (IS_ERR(node))
			return PTR_ERR(node);

		node->data = sandbox_node;

		for (j = 0; j < sandbox_node->num_links; ++j)
			icc_link_create(node, (ulong)sandbox_node->links[j]);

		plat->xlate_nodes[i] = node;
	}

	return 0;
}

static int sandbox_interconnect_unbind(struct udevice *dev)
{
	struct icc_provider *plat = dev_get_uclass_plat(dev);

	free(plat->xlate_nodes);

	return 0;
}

static const struct udevice_id sandbox_interconnect_ids[] = {
	{ .compatible = "sandbox,interconnect0", .data = (ulong)&interconnect0_data, },
	{ .compatible = "sandbox,interconnect1", .data = (ulong)&interconnect1_data, },
	{ .compatible = "sandbox,interconnect2", .data = (ulong)&interconnect2_data, },
	{ .compatible = "sandbox,interconnect3", .data = (ulong)&interconnect3_data, },
	{ .compatible = "sandbox,interconnect4", .data = (ulong)&interconnect4_data, },
	{ }
};

static struct interconnect_ops sandbox_interconnect_ops = {
	.of_xlate = sandbox_interconnect_xlate,
	.set = sandbox_interconnect_set,
	.pre_aggregate = sandbox_interconnect_pre_aggregate,
	.aggregate = sandbox_interconnect_aggregate,
};

U_BOOT_DRIVER(sandbox_interconnect) = {
	.name = "sandbox_interconnect",
	.id = UCLASS_INTERCONNECT,
	.of_match = sandbox_interconnect_ids,
	.bind = sandbox_interconnect_bind,
	.unbind = sandbox_interconnect_unbind,
	.plat_auto = sizeof(struct sandbox_interconnect_provider),
	.ops = &sandbox_interconnect_ops,
};

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2025 Linaro Limited
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <interconnect-uclass.h>
#include <dt-bindings/interconnect/qcom,icc.h>
#include <linux/err.h>
#include <dm/device_compat.h>

#include "icc-rpmh.h"
#include "bcm-voter.h"

static inline struct qcom_icc_provider *to_qcom_provider(struct udevice *dev)
{
	return dev_get_plat(dev);
}

static int qcom_icc_set(struct icc_node *src, struct icc_node *dst)
{
	struct qcom_icc_provider *qp;
	struct icc_node *node;

	if (!src)
		node = dst;
	else
		node = src;

	qp = to_qcom_provider(node->dev->parent);
	qcom_icc_bcm_voter_commit(qp->voter);

	return 0;
}

static int qcom_icc_aggregate(struct icc_node *node, u32 tag, u32 avg_bw,
			      u32 peak_bw, u32 *agg_avg, u32 *agg_peak)
{
	size_t i;
	struct qcom_icc_node *qn;

	qn = node->data;

	if (!tag)
		tag = QCOM_ICC_TAG_ALWAYS;

	for (i = 0; i < QCOM_ICC_NUM_BUCKETS; i++) {
		if (tag & BIT(i)) {
			qn->sum_avg[i] += avg_bw;
			qn->max_peak[i] = max_t(u32, qn->max_peak[i], peak_bw);
		}
	}

	*agg_avg += avg_bw;
	*agg_peak = max_t(u32, *agg_peak, peak_bw);

	return 0;
}

static void qcom_icc_pre_aggregate(struct icc_node *node)
{
	size_t i;
	struct qcom_icc_node *qn;
	struct qcom_icc_provider *qp;

	qn = node->data;
	qp = to_qcom_provider(node->dev->parent);

	for (i = 0; i < QCOM_ICC_NUM_BUCKETS; i++) {
		qn->sum_avg[i] = 0;
		qn->max_peak[i] = 0;
	}

	for (i = 0; i < qn->num_bcms; i++)
		qcom_icc_bcm_voter_add(qp->voter, qn->bcms[i]);
}

static struct icc_node *qcom_icc_xlate(struct udevice *dev,
				       const struct ofnode_phandle_args *spec)
{
	struct icc_provider *priv = dev_get_uclass_plat(dev);
	unsigned int idx = spec->args[0];

	if (idx >= priv->xlate_num_nodes) {
		pr_err("%s: invalid index %u\n", __func__, idx);
		return ERR_PTR(-EINVAL);
	}

	return priv->xlate_nodes[idx];
}

struct interconnect_ops qcom_icc_rpmh_ops = {
	.set = qcom_icc_set,
	.pre_aggregate = qcom_icc_pre_aggregate,
	.aggregate = qcom_icc_aggregate,
	.of_xlate = qcom_icc_xlate,
};

/**
 * qcom_icc_bcm_init - populates bcm aux data and connect qnodes
 * @bcm: bcm to be initialized
 * @dev: associated provider device
 *
 * Return: 0 on success, or an error code otherwise
 */
int qcom_icc_bcm_init(struct qcom_icc_bcm *bcm, struct udevice *dev)
{
	struct qcom_icc_node *qn;
	const struct bcm_db *data;
	size_t data_count;
	int i;

	/* BCM is already initialised*/
	if (bcm->addr)
		return 0;

	bcm->addr = cmd_db_read_addr(bcm->name);
	if (!bcm->addr) {
		dev_err(dev, "%s could not find RPMh address\n",
			bcm->name);
		return -EINVAL;
	}

	data = cmd_db_read_aux_data(bcm->name, &data_count);
	if (IS_ERR(data)) {
		dev_err(dev, "%s command db read error (%ld)\n",
			bcm->name, PTR_ERR(data));
		return PTR_ERR(data);
	}
	if (!data_count) {
		dev_err(dev, "%s command db missing or partial aux data\n",
			bcm->name);
		return -EINVAL;
	}

	bcm->aux_data.unit = le32_to_cpu(data->unit);
	bcm->aux_data.width = le16_to_cpu(data->width);
	bcm->aux_data.vcd = data->vcd;
	bcm->aux_data.reserved = data->reserved;
	INIT_LIST_HEAD(&bcm->list);
	INIT_LIST_HEAD(&bcm->ws_list);

	if (!bcm->vote_scale)
		bcm->vote_scale = 1000;

	/* Link Qnodes to their respective BCMs */
	for (i = 0; i < bcm->num_nodes; i++) {
		qn = bcm->nodes[i];
		qn->bcms[qn->num_bcms] = bcm;
		qn->num_bcms++;
	}

	return 0;
}

int qcom_icc_rpmh_probe(struct udevice *dev)
{
	struct qcom_icc_provider *qp = dev_get_plat(dev);
	int i;

	qp->voter = of_bcm_voter_get(qp->dev, NULL);
	if (IS_ERR(qp->voter))
		return PTR_ERR(qp->voter);

	for (i = 0; i < qp->desc->num_bcms; i++)
		qcom_icc_bcm_init(qp->desc->bcms[i], dev);

	return 0;
}

int qcom_icc_rpmh_bind(struct udevice *dev)
{
	struct icc_provider *priv = dev_get_uclass_plat(dev);
	struct qcom_icc_provider *qp = dev_get_plat(dev);
	struct qcom_icc_node * const *qnodes, *qn;
	struct icc_node *node;
	size_t num_nodes, i, j;

	qp->desc = (const struct qcom_icc_desc *)dev_get_driver_data(dev);
	if (!qp->desc)
		return -EINVAL;

	qnodes = qp->desc->nodes;
	num_nodes = qp->desc->num_nodes;

	priv->xlate_num_nodes = num_nodes;
	priv->xlate_nodes = calloc(sizeof(node), num_nodes);
	if (!priv->xlate_nodes)
		return -ENOMEM;

	qp->dev = dev;

	for (i = 0; i < num_nodes; i++) {
		qn = qnodes[i];
		if (!qn)
			continue;

		node = icc_node_create(dev, qn->id, qn->name);
		if (IS_ERR(node))
			return PTR_ERR(node);

		node->data = qn;

		for (j = 0; j < qn->num_links; j++)
			icc_link_create(node, qn->links[j]);

		priv->xlate_nodes[i] = node;
	}

	return 0;
}

int qcom_icc_rpmh_unbind(struct udevice *dev)
{
	struct icc_provider *priv = dev_get_uclass_plat(dev);

	free(priv->xlate_nodes);

	return 0;
}

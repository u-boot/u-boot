// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#include <dm.h>
#include <interconnect-uclass.h>
#include <dt-bindings/interconnect/qcom,qcs8300-rpmh.h>

#include "icc-rpmh.h"
#include "qcs8300.h"

static struct qcom_icc_node xm_ufs_mem = {
	.name = "xm_ufs_mem",
	.id = QCS8300_MASTER_UFS_MEM,
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.links = { QCS8300_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_node qns_a1noc_snoc = {
	.name = "qns_a1noc_snoc",
	.id = QCS8300_SLAVE_A1NOC_SNOC,
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.links = { QCS8300_MASTER_A1NOC_SNOC },
};

static struct qcom_icc_node qnm_aggre1_noc = {
	.name = "qnm_aggre1_noc",
	.id = QCS8300_MASTER_A1NOC_SNOC,
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.links = { QCS8300_SLAVE_SNOC_GEM_NOC_SF },
};

static struct qcom_icc_node qns_gemnoc_sf = {
	.name = "qns_gemnoc_sf",
	.id = QCS8300_SLAVE_SNOC_GEM_NOC_SF,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { QCS8300_MASTER_SNOC_SF_MEM_NOC },
};

static struct qcom_icc_node qnm_snoc_sf = {
	.name = "qnm_snoc_sf",
	.id = QCS8300_MASTER_SNOC_SF_MEM_NOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { QCS8300_SLAVE_LLCC },
};

static struct qcom_icc_node chm_apps = {
	.name = "chm_apps",
	.id = QCS8300_MASTER_APPSS_PROC,
	.channels = 4,
	.buswidth = 32,
	.num_links = 1,
	.links = { QCS8300_SLAVE_GEM_NOC_CNOC },
};

static struct qcom_icc_node qns_gem_noc_cnoc = {
	.name = "qns_gem_noc_cnoc",
	.id = QCS8300_SLAVE_GEM_NOC_CNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { QCS8300_MASTER_GEM_NOC_CNOC },
};

static struct qcom_icc_node qns_llcc = {
	.name = "qns_llcc",
	.id = QCS8300_SLAVE_LLCC,
	.channels = 4,
	.buswidth = 16,
	.num_links = 1,
	.links = { QCS8300_MASTER_LLCC },
};

static struct qcom_icc_node llcc_mc = {
	.name = "llcc_mc",
	.id = QCS8300_MASTER_LLCC,
	.channels = 8,
	.buswidth = 4,
	.num_links = 1,
	.links = { QCS8300_SLAVE_EBI1 },
};

static struct qcom_icc_node ebi = {
	.name = "ebi",
	.id = QCS8300_SLAVE_EBI1,
	.channels = 8,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qnm_gemnoc_cnoc = {
	.name = "qnm_gemnoc_cnoc",
	.id = QCS8300_MASTER_GEM_NOC_CNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { QCS8300_SLAVE_UFS_MEM_CFG },
};

static struct qcom_icc_node qhs_ufs_mem_cfg = {
	.name = "qhs_ufs_mem_cfg",
	.id = QCS8300_SLAVE_UFS_MEM_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_bcm bcm_sn3 = {
	.name = "SN3",
	.num_nodes = 2,
	.nodes = { &qns_a1noc_snoc, &qnm_aggre1_noc },
};

static struct qcom_icc_bcm bcm_sn0 = {
	.name = "SN0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_gemnoc_sf },
};

static struct qcom_icc_bcm bcm_sh0 = {
	.name = "SH0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_llcc },
};

static struct qcom_icc_bcm bcm_sh2 = {
	.name = "SH2",
	.num_nodes = 1,
	.nodes = { &chm_apps },
};

static struct qcom_icc_bcm bcm_acv = {
	.name = "ACV",
	.enable_mask = BIT(3),
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_mc0 = {
	.name = "MC0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_cn0 = {
	.name = "CN0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qnm_gemnoc_cnoc },
};

static struct qcom_icc_bcm * const aggre1_noc_bcms[] = {
	&bcm_sn3,
};

static struct qcom_icc_node * const aggre1_noc_nodes[] = {
	[MASTER_UFS_MEM] = &xm_ufs_mem,
	[SLAVE_A1NOC_SNOC] = &qns_a1noc_snoc,
};

static const struct qcom_icc_desc qcs8300_aggre1_noc = {
	.nodes = aggre1_noc_nodes,
	.num_nodes = ARRAY_SIZE(aggre1_noc_nodes),
	.bcms = aggre1_noc_bcms,
	.num_bcms = ARRAY_SIZE(aggre1_noc_bcms),
};

static struct qcom_icc_bcm * const system_noc_bcms[] = {
	&bcm_sn0,
};

static struct qcom_icc_node * const system_noc_nodes[] = {
	[MASTER_A1NOC_SNOC] = &qnm_aggre1_noc,
	[SLAVE_SNOC_GEM_NOC_SF] = &qns_gemnoc_sf,
};

static const struct qcom_icc_desc qcs8300_system_noc = {
	.nodes = system_noc_nodes,
	.num_nodes = ARRAY_SIZE(system_noc_nodes),
	.bcms = system_noc_bcms,
	.num_bcms = ARRAY_SIZE(system_noc_bcms),
};

static struct qcom_icc_bcm * const gem_noc_bcms[] = {
	&bcm_sh0,
	&bcm_sh2,
};

static struct qcom_icc_node * const gem_noc_nodes[] = {
	[MASTER_APPSS_PROC] = &chm_apps,
	[MASTER_SNOC_SF_MEM_NOC] = &qnm_snoc_sf,
	[SLAVE_GEM_NOC_CNOC] = &qns_gem_noc_cnoc,
	[SLAVE_LLCC] = &qns_llcc,
};

static const struct qcom_icc_desc qcs8300_gem_noc = {
	.nodes = gem_noc_nodes,
	.num_nodes = ARRAY_SIZE(gem_noc_nodes),
	.bcms = gem_noc_bcms,
	.num_bcms = ARRAY_SIZE(gem_noc_bcms),
};

static struct qcom_icc_bcm * const mc_virt_bcms[] = {
	&bcm_acv,
	&bcm_mc0,
};

static struct qcom_icc_node * const mc_virt_nodes[] = {
	[MASTER_LLCC] = &llcc_mc,
	[SLAVE_EBI1] = &ebi,
};

static const struct qcom_icc_desc qcs8300_mc_virt = {
	.nodes = mc_virt_nodes,
	.num_nodes = ARRAY_SIZE(mc_virt_nodes),
	.bcms = mc_virt_bcms,
	.num_bcms = ARRAY_SIZE(mc_virt_bcms),
};

static struct qcom_icc_bcm * const config_noc_bcms[] = {
	&bcm_cn0,
};

static struct qcom_icc_node * const config_noc_nodes[] = {
	[MASTER_GEM_NOC_CNOC] = &qnm_gemnoc_cnoc,
	[SLAVE_UFS_MEM_CFG] = &qhs_ufs_mem_cfg,
};

static const struct qcom_icc_desc qcs8300_config_noc = {
	.nodes = config_noc_nodes,
	.num_nodes = ARRAY_SIZE(config_noc_nodes),
	.bcms = config_noc_bcms,
	.num_bcms = ARRAY_SIZE(config_noc_bcms),
};

static const struct udevice_id qnoc_of_match[] = {
	{ .compatible = "qcom,qcs8300-aggre1-noc", .data = (ulong)&qcs8300_aggre1_noc },
	{ .compatible = "qcom,qcs8300-system-noc", .data = (ulong)&qcs8300_system_noc },
	{ .compatible = "qcom,qcs8300-gem-noc", .data = (ulong)&qcs8300_gem_noc },
	{ .compatible = "qcom,qcs8300-mc-virt", .data = (ulong)&qcs8300_mc_virt },
	{ .compatible = "qcom,qcs8300-config-noc", .data = (ulong)&qcs8300_config_noc },
	{ }
};

U_BOOT_DRIVER(qnoc_qcs8300) = {
	.name = "qnoc-qcs8300",
	.id = UCLASS_INTERCONNECT,
	.of_match = qnoc_of_match,
	.probe = qcom_icc_rpmh_probe,
	.bind = qcom_icc_rpmh_bind,
	.unbind = qcom_icc_rpmh_unbind,
	.ops = &qcom_icc_rpmh_ops,
	.plat_auto = sizeof(struct qcom_icc_provider),
};

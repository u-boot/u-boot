// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 NXP
 *
 */

#include <asm/arch/sys_proto.h>
#include <linux/bitfield.h>
#include <fuse.h>
#include <fdt_support.h>
#include <fdtdec.h>

struct periph_fuse_info {
	u32 bit_mask;
	u32 soc_type; /* 0 means for all */
	bool of_board_fix;
	int (*disable_func)(void *blob, u32 fuse_val);
};

int num_a55_cores_disabled;
int gpu_disabled;

static int delete_fdt_nodes(void *blob, const char *const nodes_path[], int size_array)
{
	int i = 0;
	int rc;
	int nodeoff;

	for (i = 0; i < size_array; i++) {
		nodeoff = fdt_path_offset(blob, nodes_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		debug("Found %s node\n", nodes_path[i]);

		rc = fdt_del_node(blob, nodeoff);
		if (rc < 0) {
			printf("Unable to delete node %s, err=%s\n",
			       nodes_path[i], fdt_strerror(rc));
		} else {
			printf("Delete node %s\n", nodes_path[i]);
		}
	}

	return 0;
}

static int disable_fdt_nodes(void *blob, const char *const nodes_path[],
			     int size_array, const char *prop, const char *value)
{
	int i = 0;
	int rc;
	int nodeoff;
	const char *status = "disabled";
	const char *prop_str;

	for (i = 0; i < size_array; i++) {
		nodeoff = fdt_path_offset(blob, nodes_path[i]);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		debug("Found %s node\n", nodes_path[i]);

		if (prop && value) {
			prop_str = fdt_stringlist_get(blob, nodeoff, prop, 0, NULL);

			if (!prop_str || strcmp(prop_str, value))
				continue;
		}

add_status:
		rc = fdt_setprop(blob, nodeoff, "status", status, strlen(status) + 1);
		if (rc) {
			if (rc == -FDT_ERR_NOSPACE) {
				rc = fdt_increase_size(blob, 512);
				if (!rc)
					goto add_status;
			}
			printf("Unable to update property %s:%s, err=%s\n",
			       nodes_path[i], "status", fdt_strerror(rc));
		} else {
			debug("Modify %s:%s disabled\n", nodes_path[i], "status");
		}
	}

	return 0;
}

static int get_cooling_device_list(void *blob, u32 nodeoff,
				   const char *const path, u32 *cooling_dev, int max_cnt)
{
		int cnt, j;

		cnt = fdtdec_get_int_array_count(blob, nodeoff, "cooling-device",
						 cooling_dev, max_cnt);
		if (cnt < 0) {
			printf("cnt incorrect, path %s, cnt = %d\n", path, cnt);
			return cnt;
		}
		if (cnt != max_cnt)
			printf("Warning: %s, cooling-device count %d\n", path, cnt);

		for (j = 0; j < cnt; j++)
			cooling_dev[j] = cpu_to_fdt32(cooling_dev[j]);

		return cnt;
}

static void disable_thermal_vpu_node(void *blob, u32 disabled_cores, u32 gpu_disabled)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/ana/cooling-maps/map0",
		"/thermal-zones/ana-thermal/cooling-maps/map0",
	};
	int num_cpus = (is_imx94() || is_imx952()) ? 4 : 6;
	u32 array_cnt = (num_cpus + 2) * 3 - (disabled_cores * 3) - (gpu_disabled * 3);
	u32 cooling_dev[array_cnt];

	int nodeoff, ret, i, cnt;

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		nodeoff = fdt_path_offset(blob, thermal_path[i]);
		if (nodeoff < 0) {
			printf("path not found %s\n", thermal_path[i]);
			continue; /* Not found, skip it */
		}

		cnt = get_cooling_device_list(blob, nodeoff,
					      thermal_path[i], cooling_dev, array_cnt);
		/* VPU map does not exist in cooling dev*/
		if (cnt <= ((num_cpus - disabled_cores) * 3 + (gpu_disabled ? 0 : 3)))
			continue;

		/* Remove  VPU it the last two nodes in the fdt ana blob */
		ret = fdt_setprop(blob, nodeoff, "cooling-device", &cooling_dev,
				  sizeof(u32) * (array_cnt - 3));

		if (ret < 0) {
			printf("Warning: %s, cooling-device setprop failed %d\n",
			       thermal_path[i], ret);
			continue;
		}

		printf("Update node %s, cooling-device prop\n", thermal_path[i]);
	}
}

static void disable_thermal_gpu_node(void *blob, u32 disabled_cores)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/ana/cooling-maps/map0",
		"/thermal-zones/ana-thermal/cooling-maps/map0",
	};
	int num_cpus = (is_imx94() || is_imx952()) ? 4 : 6;
	u32 array_cnt = (num_cpus + 2) * 3 - (disabled_cores * 3);
	u32 cooling_dev[array_cnt];
	int nodeoff, ret, i, cnt;

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		nodeoff = fdt_path_offset(blob, thermal_path[i]);
		if (nodeoff < 0) {
			printf("path not found %s\n", thermal_path[i]);
			continue; /* Not found, skip it */
		}

		cnt = get_cooling_device_list(blob, nodeoff, thermal_path[i],
					      cooling_dev, array_cnt);
		if (cnt <= (num_cpus - disabled_cores) * 3)
			continue; /* GPU map does not exist in cooling dev*/

		/* Remove GPU and VPU as these are the last two nodes in the fdt ana blob */
		ret = fdt_setprop(blob, nodeoff, "cooling-device", &cooling_dev,
				  sizeof(u32) * (array_cnt - 6));
		if (ret < 0) {
			printf("Warning: %s, cooling-device setprop failed %d\n",
			       thermal_path[i], ret);
			continue;
		}

		if (cnt == array_cnt) {
			/* Add VPU node back to ana thermal-zone. */
			ret = fdt_appendprop(blob, nodeoff, "cooling-device",
					     &cooling_dev[array_cnt - 3], sizeof(u32) * 3);
			if (ret < 0) {
				printf("Warning: %s, cooling-device appendprop failed %d\n",
				       thermal_path[i], ret);
				continue;
			}
		}

		printf("Update node %s, cooling-device prop\n", thermal_path[i]);
	}
}

static void disable_thermal_cpu_nodes(void *blob, u32 disabled_cores)
{
	static const char * const thermal_path[] = {
		"/thermal-zones/pf53_arm/cooling-maps/map0",
		"/thermal-zones/ana/cooling-maps/map0",
		"/thermal-zones/a55/cooling-maps/map0",
		"/thermal-zones/a55-thermal/cooling-maps/map0",
		"/thermal-zones/ana-thermal/cooling-maps/map0",
	};
	u32 cooling_dev[24];
	int nodeoff, ret, i, cnt;
	int prop_size = 3 * ((is_imx94() || is_imx952()) ? 4 : 6);

	for (i = 0; i < ARRAY_SIZE(thermal_path); i++) {
		nodeoff = fdt_path_offset(blob, thermal_path[i]);
		if (nodeoff < 0) {
			printf("path not found %s\n", thermal_path[i]);
			continue; /* Not found, skip it */
		}
		cnt = get_cooling_device_list(blob, nodeoff, thermal_path[i], cooling_dev, 24);

		ret = fdt_setprop(blob, nodeoff, "cooling-device", &cooling_dev,
				  sizeof(u32) * (prop_size - disabled_cores * 3));

		if (ret < 0) {
			printf("Warning: %s, cooling-device setprop failed %d\n",
			       thermal_path[i], ret);
			continue;
		}

		/* Add GPU and VPU nodes back to ana thermal-zone. */
		if (cnt > prop_size) {
			ret = fdt_appendprop(blob, nodeoff, "cooling-device",
					     &cooling_dev[prop_size],
					     sizeof(u32) * (cnt - prop_size));
			if (ret < 0) {
				printf("Warning: %s, cooling-device appendprop failed %d\n",
				       thermal_path[i], ret);
				continue;
			}
		}

		printf("Update node %s, cooling-device prop\n", thermal_path[i]);
	}
}

static int disable_ld_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_ld[] = {
		"/remoteproc",
		"/disp-mu",
		"/soc/syscon@4b070000",
		"/soc/mailbox@4b080000",
		"/soc/mailbox@4b090000",
	};

	return delete_fdt_nodes(blob, nodes_path_ld, ARRAY_SIZE(nodes_path_ld));
}

static int disable_npu_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_npu[] = {
		"/soc/imx95-neutron-remoteproc@4ab00000",
		"/soc/imx95-neutron@4ab00004",
		"/soc/neutron-remoteproc@4ab00000",
		"/soc/neutron@4ab00004",
	};

	return delete_fdt_nodes(blob, nodes_path_npu, ARRAY_SIZE(nodes_path_npu));
}

static int disable_arm_cpu_nodes(void *blob, u32 fuse_val)
{
	u32 i = 0;
	int rc;
	int nodeoff;
	char nodes_path[32];
	int num_cpus = (is_imx94() || is_imx952()) ? 4 : 6;

	num_a55_cores_disabled = 0;

	if (fuse_val & BIT(2)) /* A55C2 */
		num_a55_cores_disabled++;

	if (fuse_val & BIT(3)) /* A55C2 */
		num_a55_cores_disabled++;

	if (fuse_val & BIT(4)) /* A55C3 */
		num_a55_cores_disabled++;

	if (fuse_val & BIT(5)) /* A55C4 */
		num_a55_cores_disabled++;

	if (fuse_val & BIT(6)) /* A55C5 */
		num_a55_cores_disabled++;

	for (i = num_cpus; i > (num_cpus - num_a55_cores_disabled); i--) {
		sprintf(nodes_path, "/cpus/cpu@%u00", i - 1);

		nodeoff = fdt_path_offset(blob, nodes_path);
		if (nodeoff < 0)
			continue; /* Not found, skip it */

		debug("Found %s node\n", nodes_path);

		rc = fdt_del_node(blob, nodeoff);
		if (rc < 0) {
			printf("Unable to delete node %s, err=%s\n",
			       nodes_path, fdt_strerror(rc));
		} else {
			printf("Delete node %s\n", nodes_path);

			/* Remove node from cpu-map/cluster0 */
			sprintf(nodes_path, "/cpus/cpu-map/cluster0/core%u", i - 1);
			nodeoff = fdt_path_offset(blob, nodes_path);
			if (nodeoff < 0)
				continue; /* Not found, skip it */

			rc = fdt_del_node(blob, nodeoff);
			if (rc < 0)
				printf("Unable to delete node %s, err=%s\n",
				       nodes_path, fdt_strerror(rc));
		}
	}

	disable_thermal_cpu_nodes(blob, num_a55_cores_disabled);

	return 0;
}

static int disable_jpegdec_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_jpegdec[] = {
		"/soc/jpegdec@4c500000",
	};

	return delete_fdt_nodes(blob, nodes_path_jpegdec, ARRAY_SIZE(nodes_path_jpegdec));
}

static int disable_jpegenc_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_jpegenc[] = {
		"/soc/jpegenc@4c550000",
	};

	return delete_fdt_nodes(blob, nodes_path_jpegenc, ARRAY_SIZE(nodes_path_jpegenc));
}

static int disable_mipicsi0_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_mipicsi0[] = {
		"/soc/csi@4ad30000",
	};

	return delete_fdt_nodes(blob, nodes_path_mipicsi0, ARRAY_SIZE(nodes_path_mipicsi0));
}

static int disable_mipicsi1_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_mipicsi1[] = {
		"/soc/csi@4ad40000",
	};

	return delete_fdt_nodes(blob, nodes_path_mipicsi1, ARRAY_SIZE(nodes_path_mipicsi1));
}

static int disable_isp_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_isp[] = {
		"/soc@0/isp@4ae00000",
		"/soc/isp@4ae00000",
	};

	return delete_fdt_nodes(blob, nodes_path_isp, ARRAY_SIZE(nodes_path_isp));
}

static int disable_vpu_node(void *blob, u32 fuse_val)
{
	int ret = 0;

	static const char * const nodes_path_vpu[] = {
		"/soc/vpu-ctrl@4c4c0000",
		"/soc/vpu-ctrl@4c4f0000",
		"/soc/vpu@4c480000",
		"/soc/vpu@4c490000",
		"/soc/vpu@4c4a0000",
		"/soc/vpu@4c4b0000",
		"/soc/vpu@4c4c0000",
		"/soc/vpu@4c4d0000",
		"/soc/vpu@4c4e0000",
		"/soc/jpegdec@4c500000",
		"/soc/jpegenc@4c550000",
		"/soc/vpuenc@4c460000",
		"/soc/syscon@4c410000"
	};

	ret = delete_fdt_nodes(blob, nodes_path_vpu, ARRAY_SIZE(nodes_path_vpu));
	if (!ret)
		disable_thermal_vpu_node(blob, num_a55_cores_disabled, gpu_disabled);

	return ret;
}

static int disable_vpuenc_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_vpuenc[] = {
		"/soc/vpuenc@4c460000",
	};

	return delete_fdt_nodes(blob, nodes_path_vpuenc, ARRAY_SIZE(nodes_path_vpuenc));
}

static int disable_vpuwave511_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_vpu511[] = {
		"/soc/vpu-ctrl@4c4f0000",
		"/soc/vpu@4c4b0000",
		"/soc/vpu@4c4c0000",
		"/soc/vpu@4c4d0000",
		"/soc/vpu@4c4e0000",
	};

	return delete_fdt_nodes(blob, nodes_path_vpu511, ARRAY_SIZE(nodes_path_vpu511));
}

static int disable_gpu_node(void *blob, u32 fuse_val)
{
	int ret = 0;

	static const char * const nodes_path_gpu[] = {
		"/soc/gpu@4d900000",
		"/thermal-zones@1/ana/cooling-maps/map1/cooling-device/gpu@4d900000",
		"/thermal-zones/ana/cooling-maps/map1/cooling-device/gpu@4d900000",
		"/thermal-zones@1/ana/cooling-maps/map1/cooling-device",
		"/thermal-zones/ana/cooling-maps/map1/cooling-device",
		"/thermal-zones@1/ana/cooling-maps/map1",
		"/thermal-zones/ana/cooling-maps/map1",
	};

	ret = delete_fdt_nodes(blob, nodes_path_gpu, ARRAY_SIZE(nodes_path_gpu));
	if (!ret) {
		disable_thermal_gpu_node(blob, num_a55_cores_disabled);
		gpu_disabled = 1;
	}
	return ret;
}

static int disable_pciea_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_pciea[] = {
		"/soc/pcie@4c300000",
		"/soc/pcie-ep@4c300000"
	};

	return delete_fdt_nodes(blob, nodes_path_pciea, ARRAY_SIZE(nodes_path_pciea));
}

static int disable_pcieb_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_pcieb[] = {
		"/soc/pcie@4c380000",
		"/soc/pcie-ep@4c380000"
	};

	return delete_fdt_nodes(blob, nodes_path_pcieb, ARRAY_SIZE(nodes_path_pcieb));
}

int disable_enet10g_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_enet10g[] = {
		"/pcie@4ca00000/ethernet@10,0",
		"/soc/pcie@4ca00000/ethernet@10,0",
		"/soc/syscon@4ca00000/ethernet@10,0",
		"/soc/netc-blk-ctrl@4cde0000/pcie@4ca00000/ethernet@10,0",
	};

	return disable_fdt_nodes(blob, nodes_path_enet10g, ARRAY_SIZE(nodes_path_enet10g),
		NULL, NULL);
}

int disable_enet25g_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_enet25g[] = {
		"/soc/system-controller@4ceb0000/pcie@4ca00000/ethernet-switch@0,2/ports/port@0",
		"/soc/system-controller@4ceb0000/pcie@4ca00000/ethernet-switch@0,2/ports/port@1",
	};

	return disable_fdt_nodes(blob, nodes_path_enet25g, ARRAY_SIZE(nodes_path_enet25g),
		"phy-mode", "sgmii");
}

int disable_mipidsi_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_mipidsi[] = {
		"/soc/dsi@4acf0000",
		"/soc/syscon@4acf0000",
		"/soc/dsi@4b060000",
		"/soc/phy@4b110000",
	};

	return delete_fdt_nodes(blob, nodes_path_mipidsi, ARRAY_SIZE(nodes_path_mipidsi));
}

int disable_dpu_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_dpu[] = {
		"/soc/bridge@4b0d0000/channel@0/port@0/endpoint",
		"/soc/bridge@4b0d0000/channel@0/port@1/endpoint",
		"/soc/bridge@4b0d0000/channel@1/port@0/endpoint",
		"/soc/bridge@4b0d0000/channel@1/port@1/endpoint",
		"/soc/display-controller@4b400000/ports/port@0/endpoint",
		"/soc/display-controller@4b400000/ports/port@1/endpoint",
		"/soc/display-controller@4b400000",
		"/soc/syscon@4b010000/bridge@8/ports/port@0/endpoint",
		"/soc/syscon@4b010000/bridge@8/ports/port@1/endpoint",
		"/soc/syscon@4b010000/bridge@8/ports/port@2/endpoint@0",
		"/soc/syscon@4b010000/bridge@8/ports/port@2/endpoint@1",
		"/soc/syscon@4b010000/bridge@8/ports/port@3/endpoint@0",
		"/soc/syscon@4b010000/bridge@8/ports/port@3/endpoint@1",
		"/soc/syscon@4b010000/bridge@8",
		"/soc/syscon@4b010000",
		"/soc/syscon@4b0a0000",
		"/soc/interrupt-controller@4b0b0000",
		"/soc/bridge@4b0d0000"
	};

	return delete_fdt_nodes(blob, nodes_path_dpu, ARRAY_SIZE(nodes_path_dpu));
}

int disable_lvds_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_lvds[] = {
		"/soc/syscon@4b0c0000/ldb@4/channel@0",
		"/soc/syscon@4b0c0000/phy@8",
		"/soc/syscon@4b0c0000/ldb@4/channel@1",
		"/soc/syscon@4b0c0000/phy@c",
		"/soc/syscon@4b0c0000"
	};

	return delete_fdt_nodes(blob, nodes_path_lvds, ARRAY_SIZE(nodes_path_lvds));
}

int disable_cm70_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_cm70[] = {
		"/reserved-memory/vdev0vring0@82000000",
		"/reserved-memory/vdev0vring1@82008000",
		"/reserved-memory/vdev1vring0@82010000",
		"/reserved-memory/vdev1vring1@82018000",
		"/reserved-memory/rsc-table@82220000",
		"/reserved-memory/vdevbuffer@82020000",
		"/imx943-cm70",
	};

	return delete_fdt_nodes(blob, nodes_path_cm70, ARRAY_SIZE(nodes_path_cm70));
}

int disable_cm71_node(void *blob, u32 fuse_val)
{
	static const char * const nodes_path_cm71[] = {
		"/reserved-memory/vdev0vring0@84000000",
		"/reserved-memory/vdev0vring1@84008000",
		"/reserved-memory/vdev1vring0@84010000",
		"/reserved-memory/vdev1vring1@84018000",
		"/reserved-memory/rsc-table@84220000",
		"/reserved-memory/vdevbuffer@84020000",
		"/imx943-cm71",
	};

	return delete_fdt_nodes(blob, nodes_path_cm71, ARRAY_SIZE(nodes_path_cm71));
}

/* There is order dependency between cpu->gpu->vpu */
struct periph_fuse_info f17_grp[] = {
	{ BIT(30), MXC_CPU_IMX952, false, disable_ld_node },
};

struct periph_fuse_info f18_grp[] = {
	{ BIT(0), 0, false, disable_npu_node },
	{ GENMASK(6, 2), 0, false, disable_arm_cpu_nodes },
	{ BIT(9), 0, true, disable_cm70_node },
	{ BIT(17), MXC_CPU_IMX94, true, disable_cm71_node },
	{ BIT(22), 0, true, disable_dpu_node },
	{ BIT(27), 0, true, disable_lvds_node },
	{ BIT(29), 0, false, disable_isp_node },
};

struct periph_fuse_info f19_grp[] = {
	{ BIT(6), 0, true, disable_pciea_node },
	{ BIT(7), 0, true, disable_pcieb_node },
	{ BIT(17), 0, false, disable_gpu_node },
	{ BIT(18), 0, false, disable_vpu_node },
	{ BIT(19), 0, false, disable_jpegenc_node },
	{ BIT(20), 0, false, disable_jpegdec_node },
	{ BIT(22), 0, false, disable_mipicsi0_node },
	{ BIT(23), 0, false, disable_mipicsi1_node },
	{ BIT(24), 0, true, disable_mipidsi_node },
	{ BIT(26), 0, false, disable_vpuenc_node },
	{ BIT(27), 0, false, disable_vpuwave511_node },
};

struct periph_fuse_info f20_grp[] = {
	{ BIT(12), MXC_CPU_IMX95, true, disable_enet10g_node },
	{ BIT(12), MXC_CPU_IMX94, true, disable_enet25g_node },
};

static void ft_disable_periph(void *blob, u32 fuse_bank, u32 fuse_word,
			      struct periph_fuse_info *info, u32 info_num,
			      bool board_fix_fdt)
{
	int i, ret;
	u32 val = 0;

	ret = fuse_read(fuse_bank, fuse_word, &val);
	if (ret)
		return;

	for (i = 0; i < info_num; i++) {
		if (val & info[i].bit_mask) {
			if (board_fix_fdt && !info[i].of_board_fix)
				continue;

			if (!info[i].soc_type || is_cpu_type(info[i].soc_type))
				info[i].disable_func(blob, val);
		}
	}
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	/* Common peripheral disable fuse process */
	ft_disable_periph(blob, 2, 1, f17_grp, ARRAY_SIZE(f17_grp), false);
	ft_disable_periph(blob, 2, 2, f18_grp, ARRAY_SIZE(f18_grp), false);
	ft_disable_periph(blob, 2, 3, f19_grp, ARRAY_SIZE(f19_grp), false);
	ft_disable_periph(blob, 2, 4, f20_grp, ARRAY_SIZE(f20_grp), false);

	return 0;
}

/* Fix uboot dtb based on fuses. */
#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP) && !IS_ENABLED(CONFIG_XPL_BUILD)
int imx9_uboot_fixup_by_fuse(void *fdt)
{
	ft_disable_periph(fdt, 2, 2, f18_grp, ARRAY_SIZE(f18_grp), true);
	ft_disable_periph(fdt, 2, 3, f19_grp, ARRAY_SIZE(f19_grp), true);
	ft_disable_periph(fdt, 2, 4, f20_grp, ARRAY_SIZE(f20_grp), true);

	return 0;
}
#endif

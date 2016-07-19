/*
 * Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <linux/sizes.h>
#include <linux/kernel.h>
#include <asm/psci.h>
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#include <asm/armv8/sec_firmware.h>
#endif

int fdt_psci(void *fdt)
{
#if defined(CONFIG_ARMV8_PSCI) || defined(CONFIG_ARMV7_PSCI)
	int nodeoff;
	unsigned int psci_ver = 0;
	char *psci_compt;
	int tmp;

	nodeoff = fdt_path_offset(fdt, "/cpus");
	if (nodeoff < 0) {
		printf("couldn't find /cpus\n");
		return nodeoff;
	}

	/* add 'enable-method = "psci"' to each cpu node */
	for (tmp = fdt_first_subnode(fdt, nodeoff);
	     tmp >= 0;
	     tmp = fdt_next_subnode(fdt, tmp)) {
		const struct fdt_property *prop;
		int len;

		prop = fdt_get_property(fdt, tmp, "device_type", &len);
		if (!prop)
			continue;
		if (len < 4)
			continue;
		if (strcmp(prop->data, "cpu"))
			continue;

		/*
		 * Not checking rv here, our approach is to skip over errors in
		 * individual cpu nodes, hopefully some of the nodes are
		 * processed correctly and those will boot
		 */
		fdt_setprop_string(fdt, tmp, "enable-method", "psci");
	}

	/*
	 * The PSCI node might be called "/psci" or might be called something
	 * else but contain either of the compatible strings
	 * "arm,psci"/"arm,psci-0.2"
	 */
	nodeoff = fdt_path_offset(fdt, "/psci");
	if (nodeoff >= 0)
		goto init_psci_node;

	nodeoff = fdt_node_offset_by_compatible(fdt, -1, "arm,psci");
	if (nodeoff >= 0)
		goto init_psci_node;

	nodeoff = fdt_node_offset_by_compatible(fdt, -1, "arm,psci-0.2");
	if (nodeoff >= 0)
		goto init_psci_node;

	nodeoff = fdt_node_offset_by_compatible(fdt, -1, "arm,psci-1.0");
	if (nodeoff >= 0)
		goto init_psci_node;

	nodeoff = fdt_path_offset(fdt, "/");
	if (nodeoff < 0)
		return nodeoff;

	nodeoff = fdt_add_subnode(fdt, nodeoff, "psci");
	if (nodeoff < 0)
		return nodeoff;

init_psci_node:
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
	psci_ver = sec_firmware_support_psci_version();
#endif
	switch (psci_ver) {
	case 0x00010000:
		psci_compt = "arm,psci-1.0";
		break;
	case 0x00000002:
		psci_compt = "arm,psci-0.2";
		break;
	default:
		psci_compt = "arm,psci";
		break;
	}

	tmp = fdt_setprop_string(fdt, nodeoff, "compatible", psci_compt);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_string(fdt, nodeoff, "method", "smc");
	if (tmp)
		return tmp;

#ifdef CONFIG_ARMV7_PSCI
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_suspend",
				ARM_PSCI_FN_CPU_SUSPEND);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_off", ARM_PSCI_FN_CPU_OFF);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_on", ARM_PSCI_FN_CPU_ON);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "migrate", ARM_PSCI_FN_MIGRATE);
	if (tmp)
		return tmp;
#endif
#endif
	return 0;
}

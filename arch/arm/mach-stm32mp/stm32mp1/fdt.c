// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019-2020, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <fdtdec.h>
#include <fdt_support.h>
#include <log.h>
#include <tee.h>
#include <mach/stm32.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/pinctrl/stm32-pinfunc.h>
#include <linux/io.h>

#define STM32MP13_FDCAN_BASE	0x4400F000
#define STM32MP13_ADC1_BASE	0x48003000
#define STM32MP13_TSC_BASE	0x5000B000
#define STM32MP13_CRYP_BASE	0x54002000
#define STM32MP13_ETH2_BASE	0x5800E000
#define STM32MP13_DCMIPP_BASE	0x5A000000
#define STM32MP13_LTDC_BASE	0x5A010000

#define STM32MP15_FDCAN_BASE	0x4400e000
#define STM32MP15_CRYP2_BASE	0x4c005000
#define STM32MP15_CRYP1_BASE	0x54001000
#define STM32MP15_GPU_BASE	0x59000000
#define STM32MP15_DSI_BASE	0x5a000000

/* fdt helper */
static bool fdt_disable_subnode_by_address(void *fdt, int offset, u32 addr)
{
	int node;
	fdt_addr_t regs;

	for (node = fdt_first_subnode(fdt, offset);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		regs = fdtdec_get_addr(fdt, node, "reg");
		if (addr == regs) {
			if (fdtdec_get_is_enabled(fdt, node)) {
				fdt_status_disabled(fdt, node);

				return true;
			}
			return false;
		}
	}

	return false;
}

/* deactivate all the cpu except core 0 */
static void stm32_fdt_fixup_cpu(void *blob, char *name)
{
	int off;
	u32 reg;

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		log_warning("%s: couldn't find /cpus node\n", __func__);
		return;
	}

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		reg = fdtdec_get_addr(blob, off, "reg");
		if (reg != 0) {
			fdt_del_node(blob, off);
			log_notice("FDT: cpu %d node remove for %s\n",
				   reg, name);
			/* after delete we can't trust the offsets anymore */
			off = -1;
		}
		off = fdt_node_offset_by_prop_value(blob, off,
						    "device_type", "cpu", 4);
	}
}

static void stm32_fdt_disable(void *fdt, int offset, u32 addr,
			      const char *string, const char *name)
{
	if (fdt_disable_subnode_by_address(fdt, offset, addr))
		log_notice("FDT: %s@%08x node disabled for %s\n",
			   string, addr, name);
}

static void stm32_fdt_disable_optee(void *blob)
{
	int off, node;

	/* Delete "optee" firmware node */
	off = fdt_node_offset_by_compatible(blob, -1, "linaro,optee-tz");
	if (off >= 0 && fdtdec_get_is_enabled(blob, off))
		fdt_del_node(blob, off);

	/* Delete "optee@..." reserved-memory node */
	off = fdt_path_offset(blob, "/reserved-memory/");
	if (off < 0)
		return;
	for (node = fdt_first_subnode(blob, off);
	     node >= 0;
	     node = fdt_next_subnode(blob, node)) {
		if (strncmp(fdt_get_name(blob, node, NULL), "optee@", 6))
			continue;

		if (fdt_del_node(blob, node))
			printf("Failed to remove optee reserved-memory node\n");
	}
}

static void stm32mp13_fdt_fixup(void *blob, int soc, u32 cpu, char *name)
{
	switch (cpu) {
	case CPU_STM32MP131Fxx:
	case CPU_STM32MP131Dxx:
	case CPU_STM32MP131Cxx:
	case CPU_STM32MP131Axx:
		stm32_fdt_disable(blob, soc, STM32MP13_FDCAN_BASE, "can", name);
		stm32_fdt_disable(blob, soc, STM32MP13_ADC1_BASE, "adc", name);
		fallthrough;
	case CPU_STM32MP133Fxx:
	case CPU_STM32MP133Dxx:
	case CPU_STM32MP133Cxx:
	case CPU_STM32MP133Axx:
		stm32_fdt_disable(blob, soc, STM32MP13_LTDC_BASE, "ltdc", name);
		stm32_fdt_disable(blob, soc, STM32MP13_DCMIPP_BASE, "dcmipp",
				  name);
		stm32_fdt_disable(blob, soc, STM32MP13_TSC_BASE, "tsc", name);
		break;
	default:
		break;
	}

	switch (cpu) {
	case CPU_STM32MP135Dxx:
	case CPU_STM32MP135Axx:
	case CPU_STM32MP133Dxx:
	case CPU_STM32MP133Axx:
	case CPU_STM32MP131Dxx:
	case CPU_STM32MP131Axx:
		stm32_fdt_disable(blob, soc, STM32MP13_CRYP_BASE, "cryp", name);
		break;
	default:
		break;
	}
}

static void stm32mp15_fdt_fixup(void *blob, int soc, u32 cpu, char *name)
{
	u32 pkg;

	switch (cpu) {
	case CPU_STM32MP151Fxx:
	case CPU_STM32MP151Dxx:
	case CPU_STM32MP151Cxx:
	case CPU_STM32MP151Axx:
		stm32_fdt_fixup_cpu(blob, name);
		/* after cpu delete we can't trust the soc offsets anymore */
		soc = fdt_path_offset(blob, "/soc");
		stm32_fdt_disable(blob, soc, STM32MP15_FDCAN_BASE, "can", name);
		fallthrough;
	case CPU_STM32MP153Fxx:
	case CPU_STM32MP153Dxx:
	case CPU_STM32MP153Cxx:
	case CPU_STM32MP153Axx:
		stm32_fdt_disable(blob, soc, STM32MP15_GPU_BASE, "gpu", name);
		stm32_fdt_disable(blob, soc, STM32MP15_DSI_BASE, "dsi", name);
		break;
	default:
		break;
	}
	switch (cpu) {
	case CPU_STM32MP157Dxx:
	case CPU_STM32MP157Axx:
	case CPU_STM32MP153Dxx:
	case CPU_STM32MP153Axx:
	case CPU_STM32MP151Dxx:
	case CPU_STM32MP151Axx:
		stm32_fdt_disable(blob, soc, STM32MP15_CRYP1_BASE, "cryp",
				  name);
		stm32_fdt_disable(blob, soc, STM32MP15_CRYP2_BASE, "cryp",
				  name);
		break;
	default:
		break;
	}
	switch (get_cpu_package()) {
	case STM32MP15_PKG_AA_LBGA448:
		pkg = STM32MP_PKG_AA;
		break;
	case STM32MP15_PKG_AB_LBGA354:
		pkg = STM32MP_PKG_AB;
		break;
	case STM32MP15_PKG_AC_TFBGA361:
		pkg = STM32MP_PKG_AC;
		break;
	case STM32MP15_PKG_AD_TFBGA257:
		pkg = STM32MP_PKG_AD;
		break;
	default:
		pkg = 0;
		break;
	}
	if (pkg) {
		do_fixup_by_compat_u32(blob, "st,stm32mp157-pinctrl",
				       "st,package", pkg, false);
		do_fixup_by_compat_u32(blob, "st,stm32mp157-z-pinctrl",
				       "st,package", pkg, false);
	}
}

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret = 0;
	int soc;
	u32 cpu;
	char name[SOC_NAME_SIZE];

	soc = fdt_path_offset(blob, "/soc");
	/* when absent, nothing to do */
	if (soc == -FDT_ERR_NOTFOUND)
		return 0;
	if (soc < 0)
		return soc;

	/* MPUs Part Numbers and name*/
	cpu = get_cpu_type();
	get_soc_name(name);

	if (IS_ENABLED(CONFIG_STM32MP13X))
		stm32mp13_fdt_fixup(blob, soc, cpu, name);

	if (IS_ENABLED(CONFIG_STM32MP15X)) {
		stm32mp15_fdt_fixup(blob, soc, cpu, name);

		/*
		 * TEMP: remove OP-TEE nodes in kernel device tree
		 *       copied from U-Boot device tree by optee_copy_fdt_nodes
		 *       when OP-TEE is not detected (probe failed)
		 * these OP-TEE nodes are present in <board>-u-boot.dtsi
		 * under CONFIG_STM32MP15x_STM32IMAGE only for compatibility
		 * when FIP is not used by TF-A
		 */
		if (IS_ENABLED(CONFIG_STM32MP15X_STM32IMAGE) &&
		    !tee_find_device(NULL, NULL, NULL, NULL))
			stm32_fdt_disable_optee(blob);
	}

	return ret;
}

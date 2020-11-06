// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019-2020, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <fdt_support.h>
#include <log.h>
#include <tee.h>
#include <asm/arch/sys_proto.h>
#include <dt-bindings/pinctrl/stm32-pinfunc.h>
#include <linux/io.h>

#define ETZPC_DECPROT(n)	(STM32_ETZPC_BASE + 0x10 + 4 * (n))
#define ETZPC_DECPROT_NB	6

#define DECPROT_MASK		0x03
#define NB_PROT_PER_REG		0x10
#define DECPROT_NB_BITS		2

#define DECPROT_SECURED		0x00
#define DECPROT_WRITE_SECURE	0x01
#define DECPROT_MCU_ISOLATION	0x02
#define DECPROT_NON_SECURED	0x03

#define ETZPC_RESERVED		0xffffffff

#define STM32_FDCAN_BASE	0x4400e000
#define STM32_CRYP2_BASE	0x4c005000
#define STM32_CRYP1_BASE	0x54001000
#define STM32_GPU_BASE		0x59000000
#define STM32_DSI_BASE		0x5a000000

static const u32 stm32mp1_ip_addr[] = {
	0x5c008000,	/* 00 stgenc */
	0x54000000,	/* 01 bkpsram */
	0x5c003000,	/* 02 iwdg1 */
	0x5c000000,	/* 03 usart1 */
	0x5c001000,	/* 04 spi6 */
	0x5c002000,	/* 05 i2c4 */
	ETZPC_RESERVED,	/* 06 reserved */
	0x54003000,	/* 07 rng1 */
	0x54002000,	/* 08 hash1 */
	STM32_CRYP1_BASE,	/* 09 cryp1 */
	0x5a003000,	/* 0A ddrctrl */
	0x5a004000,	/* 0B ddrphyc */
	0x5c009000,	/* 0C i2c6 */
	ETZPC_RESERVED,	/* 0D reserved */
	ETZPC_RESERVED,	/* 0E reserved */
	ETZPC_RESERVED,	/* 0F reserved */
	0x40000000,	/* 10 tim2 */
	0x40001000,	/* 11 tim3 */
	0x40002000,	/* 12 tim4 */
	0x40003000,	/* 13 tim5 */
	0x40004000,	/* 14 tim6 */
	0x40005000,	/* 15 tim7 */
	0x40006000,	/* 16 tim12 */
	0x40007000,	/* 17 tim13 */
	0x40008000,	/* 18 tim14 */
	0x40009000,	/* 19 lptim1 */
	0x4000a000,	/* 1A wwdg1 */
	0x4000b000,	/* 1B spi2 */
	0x4000c000,	/* 1C spi3 */
	0x4000d000,	/* 1D spdifrx */
	0x4000e000,	/* 1E usart2 */
	0x4000f000,	/* 1F usart3 */
	0x40010000,	/* 20 uart4 */
	0x40011000,	/* 21 uart5 */
	0x40012000,	/* 22 i2c1 */
	0x40013000,	/* 23 i2c2 */
	0x40014000,	/* 24 i2c3 */
	0x40015000,	/* 25 i2c5 */
	0x40016000,	/* 26 cec */
	0x40017000,	/* 27 dac */
	0x40018000,	/* 28 uart7 */
	0x40019000,	/* 29 uart8 */
	ETZPC_RESERVED,	/* 2A reserved */
	ETZPC_RESERVED,	/* 2B reserved */
	0x4001c000,	/* 2C mdios */
	ETZPC_RESERVED,	/* 2D reserved */
	ETZPC_RESERVED,	/* 2E reserved */
	ETZPC_RESERVED,	/* 2F reserved */
	0x44000000,	/* 30 tim1 */
	0x44001000,	/* 31 tim8 */
	ETZPC_RESERVED,	/* 32 reserved */
	0x44003000,	/* 33 usart6 */
	0x44004000,	/* 34 spi1 */
	0x44005000,	/* 35 spi4 */
	0x44006000,	/* 36 tim15 */
	0x44007000,	/* 37 tim16 */
	0x44008000,	/* 38 tim17 */
	0x44009000,	/* 39 spi5 */
	0x4400a000,	/* 3A sai1 */
	0x4400b000,	/* 3B sai2 */
	0x4400c000,	/* 3C sai3 */
	0x4400d000,	/* 3D dfsdm */
	STM32_FDCAN_BASE,	/* 3E tt_fdcan */
	ETZPC_RESERVED,	/* 3F reserved */
	0x50021000,	/* 40 lptim2 */
	0x50022000,	/* 41 lptim3 */
	0x50023000,	/* 42 lptim4 */
	0x50024000,	/* 43 lptim5 */
	0x50027000,	/* 44 sai4 */
	0x50025000,	/* 45 vrefbuf */
	0x4c006000,	/* 46 dcmi */
	0x4c004000,	/* 47 crc2 */
	0x48003000,	/* 48 adc */
	0x4c002000,	/* 49 hash2 */
	0x4c003000,	/* 4A rng2 */
	STM32_CRYP2_BASE,	/* 4B cryp2 */
	ETZPC_RESERVED,	/* 4C reserved */
	ETZPC_RESERVED,	/* 4D reserved */
	ETZPC_RESERVED,	/* 4E reserved */
	ETZPC_RESERVED,	/* 4F reserved */
	ETZPC_RESERVED,	/* 50 sram1 */
	ETZPC_RESERVED,	/* 51 sram2 */
	ETZPC_RESERVED,	/* 52 sram3 */
	ETZPC_RESERVED,	/* 53 sram4 */
	ETZPC_RESERVED,	/* 54 retram */
	0x49000000,	/* 55 otg */
	0x48004000,	/* 56 sdmmc3 */
	0x48005000,	/* 57 dlybsd3 */
	0x48000000,	/* 58 dma1 */
	0x48001000,	/* 59 dma2 */
	0x48002000,	/* 5A dmamux */
	0x58002000,	/* 5B fmc */
	0x58003000,	/* 5C qspi */
	0x58004000,	/* 5D dlybq */
	0x5800a000,	/* 5E eth */
	ETZPC_RESERVED,	/* 5F reserved */
};

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

static int stm32_fdt_fixup_etzpc(void *fdt, int soc_node)
{
	const u32 *array;
	int array_size, i;
	int offset, shift;
	u32 addr, status, decprot[ETZPC_DECPROT_NB];

	array = stm32mp1_ip_addr;
	array_size = ARRAY_SIZE(stm32mp1_ip_addr);

	for (i = 0; i < ETZPC_DECPROT_NB; i++)
		decprot[i] = readl(ETZPC_DECPROT(i));

	for (i = 0; i < array_size; i++) {
		offset = i / NB_PROT_PER_REG;
		shift = (i % NB_PROT_PER_REG) * DECPROT_NB_BITS;
		status = (decprot[offset] >> shift) & DECPROT_MASK;
		addr = array[i];

		log_debug("ETZPC: 0x%08x decprot %d=%d\n", addr, i, status);

		if (addr == ETZPC_RESERVED ||
		    status == DECPROT_NON_SECURED)
			continue;

		if (fdt_disable_subnode_by_address(fdt, soc_node, addr))
			log_notice("ETZPC: 0x%08x node disabled, decprot %d=%d\n",
				   addr, i, status);
	}

	return 0;
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

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret = 0;
	int soc;
	u32 pkg, cpu;
	char name[SOC_NAME_SIZE];

	soc = fdt_path_offset(blob, "/soc");
	if (soc < 0)
		return soc;

	if (CONFIG_IS_ENABLED(STM32_ETZPC)) {
		ret = stm32_fdt_fixup_etzpc(blob, soc);
		if (ret)
			return ret;
	}

	/* MPUs Part Numbers and name*/
	cpu = get_cpu_type();
	get_soc_name(name);

	switch (cpu) {
	case CPU_STM32MP151Fxx:
	case CPU_STM32MP151Dxx:
	case CPU_STM32MP151Cxx:
	case CPU_STM32MP151Axx:
		stm32_fdt_fixup_cpu(blob, name);
		/* after cpu delete we can't trust the soc offsets anymore */
		soc = fdt_path_offset(blob, "/soc");
		stm32_fdt_disable(blob, soc, STM32_FDCAN_BASE, "can", name);
		/* fall through */
	case CPU_STM32MP153Fxx:
	case CPU_STM32MP153Dxx:
	case CPU_STM32MP153Cxx:
	case CPU_STM32MP153Axx:
		stm32_fdt_disable(blob, soc, STM32_GPU_BASE, "gpu", name);
		stm32_fdt_disable(blob, soc, STM32_DSI_BASE, "dsi", name);
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
		stm32_fdt_disable(blob, soc, STM32_CRYP1_BASE, "cryp", name);
		stm32_fdt_disable(blob, soc, STM32_CRYP2_BASE, "cryp", name);
		break;
	default:
		break;
	}

	switch (get_cpu_package()) {
	case PKG_AA_LBGA448:
		pkg = STM32MP_PKG_AA;
		break;
	case PKG_AB_LBGA354:
		pkg = STM32MP_PKG_AB;
		break;
	case PKG_AC_TFBGA361:
		pkg = STM32MP_PKG_AC;
		break;
	case PKG_AD_TFBGA257:
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

	if (!CONFIG_IS_ENABLED(OPTEE) ||
	    !tee_find_device(NULL, NULL, NULL, NULL))
		stm32_fdt_disable_optee(blob);

	return ret;
}

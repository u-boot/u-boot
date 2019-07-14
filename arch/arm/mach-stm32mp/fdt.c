// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <fdt_support.h>
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
	0x54001000,	/* 09 cryp1 */
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
	0x4400e000,	/* 3E tt_fdcan */
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
	0x4c005000,	/* 4B cryp2 */
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

	for (node = fdt_first_subnode(fdt, offset);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		if (addr == (u32)fdt_getprop(fdt, node, "reg", 0)) {
			if (fdtdec_get_is_enabled(fdt, node)) {
				fdt_status_disabled(fdt, node);

				return true;
			}
			return false;
		}
	}

	return false;
}

static int stm32_fdt_fixup_etzpc(void *fdt)
{
	const u32 *array;
	int array_size, i;
	int soc_node, offset, shift;
	u32 addr, status, decprot[ETZPC_DECPROT_NB];

	array = stm32mp1_ip_addr;
	array_size = ARRAY_SIZE(stm32mp1_ip_addr);

	for (i = 0; i < ETZPC_DECPROT_NB; i++)
		decprot[i] = readl(ETZPC_DECPROT(i));

	soc_node = fdt_path_offset(fdt, "/soc");
	if (soc_node < 0)
		return soc_node;

	for (i = 0; i < array_size; i++) {
		offset = i / NB_PROT_PER_REG;
		shift = (i % NB_PROT_PER_REG) * DECPROT_NB_BITS;
		status = (decprot[offset] >> shift) & DECPROT_MASK;
		addr = array[i];

		debug("ETZPC: 0x%08x decprot %d=%d\n", addr, i, status);

		if (addr == ETZPC_RESERVED ||
		    status == DECPROT_NON_SECURED)
			continue;

		if (fdt_disable_subnode_by_address(fdt, soc_node, addr))
			printf("ETZPC: 0x%08x node disabled, decprot %d=%d\n",
			       addr, i, status);
	}

	return 0;
}

/*
 * This function is called right before the kernel is booted. "blob" is the
 * device tree that will be passed to the kernel.
 */
int ft_system_setup(void *blob, bd_t *bd)
{
	int ret = 0;
	u32 pkg;

	if (CONFIG_IS_ENABLED(STM32_ETZPC)) {
		ret = stm32_fdt_fixup_etzpc(blob);
		if (ret)
			return ret;
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

	return ret;
}

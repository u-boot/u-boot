// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */
#include <common.h>
#include <asm/arch/eeprom.h>
#include <asm/csr.h>
#include <asm/sections.h>
#include <dm.h>
#include <linux/sizes.h>
#include <log.h>
#include <init.h>

#define CSR_U74_FEATURE_DISABLE	0x7c1
#define L2_LIM_MEM_END	0x81FFFFFUL

DECLARE_GLOBAL_DATA_PTR;

static bool check_ddr_size(phys_size_t size)
{
	switch (size) {
	case SZ_2:
	case SZ_4:
	case SZ_8:
	case SZ_16:
		return true;
	default:
		return false;
	}
}

int spl_soc_init(void)
{
	int ret;
	struct udevice *dev;
	phys_size_t size;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		return ret;

	/* Read the definition of the DDR size from eeprom, and if not,
	 * use the definition in DT
	 */
	size = (get_ddr_size_from_eeprom() >> 16) & 0xFF;
	if (check_ddr_size(size))
		gd->ram_size = size << 30;

	/* DDR init */
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}

	return 0;
}

void harts_early_init(void)
{
	ulong *ptr;
	u8 *tmp;
	ulong len, remain;
	/*
	 * Feature Disable CSR
	 *
	 * Clear feature disable CSR to '0' to turn on all features for
	 * each core. This operation must be in M-mode.
	 */
	if (CONFIG_IS_ENABLED(RISCV_MMODE))
		csr_write(CSR_U74_FEATURE_DISABLE, 0);

	/* clear L2 LIM  memory
	 * set __bss_end to 0x81FFFFF region to zero
	 * The L2 Cache Controller supports ECC. ECC is applied to SRAM.
	 * If it is not cleared, the ECC part is invalid, and an ECC error
	 * will be reported when reading data.
	 */
	ptr = (ulong *)&__bss_end;
	len = L2_LIM_MEM_END - (ulong)&__bss_end;
	remain = len % sizeof(ulong);
	len /= sizeof(ulong);

	while (len--)
		*ptr++ = 0;

	/* clear the remain bytes */
	if (remain) {
		tmp = (u8 *)ptr;
		while (remain--)
			*tmp++ = 0;
	}
}

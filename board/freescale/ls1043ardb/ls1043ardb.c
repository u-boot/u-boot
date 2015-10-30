/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	static const char *freq[3] = {"100.00MHZ", "156.25MHZ"};
#ifndef CONFIG_SD_BOOT
	u8 cfg_rcw_src1, cfg_rcw_src2;
	u32 cfg_rcw_src;
#endif
	u32 sd1refclk_sel;

	printf("Board: LS1043ARDB, boot from ");

#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#else
	cfg_rcw_src1 = CPLD_READ(cfg_rcw_src1);
	cfg_rcw_src2 = CPLD_READ(cfg_rcw_src2);
	cpld_rev_bit(&cfg_rcw_src1);
	cfg_rcw_src = cfg_rcw_src1;
	cfg_rcw_src = (cfg_rcw_src << 1) | cfg_rcw_src2;

	if (cfg_rcw_src == 0x25)
		printf("vBank %d\n", CPLD_READ(vbank));
	else if (cfg_rcw_src == 0x106)
		puts("NAND\n");
	else
		printf("Invalid setting of SW4\n");
#endif

	printf("CPLD:  V%x.%x\nPCBA:  V%x.0\n", CPLD_READ(cpld_ver),
	       CPLD_READ(cpld_ver_sub), CPLD_READ(pcba_ver));

	puts("SERDES Reference Clocks:\n");
	sd1refclk_sel = CPLD_READ(sd1refclk_sel);
	printf("SD1_CLK1 = %s, SD1_CLK2 = %s\n", freq[sd1refclk_sel], freq[0]);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = initdram(0);

	return 0;
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();
	return 0;
}

int board_init(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CONFIG_SYS_CCI400_ADDR;

	/*
	 * Set CCI-400 control override register to enable barrier
	 * transaction
	 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

#ifdef CONFIG_FSL_IFC
	init_final_memctl_regs();
#endif

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

	return 0;
}

int config_board_mux(void)
{
	return 0;
}

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
	config_board_mux();

	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif
	return 0;
}

u8 flash_read8(void *addr)
{
	return __raw_readb(addr + 1);
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

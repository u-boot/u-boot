/* GRLIB Memory controller setup. The register values are used
 * from the associated low level assembler routine implemented
 * in memcfg_low.S.
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <ambapp.h>
#include "memcfg.h"
#include <config.h>

#ifdef CONFIG_SYS_GRLIB_ESA_MCTRL1
struct mctrl_setup esa_mctrl1_cfg = {
	.reg_mask = 0x7,
	.regs = {
		{
			.mask = 0x00000300,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG1,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG2,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL1_CFG3,
		},
	}
};
#ifdef CONFIG_SYS_GRLIB_ESA_MCTRL2
struct mctrl_setup esa_mctrl2_cfg = {
	.reg_mask = 0x7,
	.regs = {
		{
			.mask = 0x00000300,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL2_CFG1,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL2_CFG2,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_ESA_MCTRL2_CFG3,
		},
	}
};
#endif
#endif

#ifdef CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1
struct mctrl_setup gaisler_ftmctrl1_cfg = {
	.reg_mask = 0x7,
	.regs = {
		{
			.mask = 0x00000300,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG1,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG2,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1_CFG3,
		},
	}
};
#ifdef CONFIG_SYS_GRLIB_GAISLER_FTMCTRL2
struct mctrl_setup gaisler_ftmctrl2_cfg = {
	.reg_mask = 0x7,
	.regs = {
		{
			.mask = 0x00000300,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL2_CFG1,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL2_CFG2,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_FTMCTRL2_CFG3,
		},
	}
};
#endif
#endif

#ifdef CONFIG_SYS_GRLIB_GAISLER_SDCTRL1
struct mctrl_setup gaisler_sdctrl1_cfg = {
	.reg_mask = 0x1,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_SDCTRL1_CTRL,
		},
	}
};
#ifdef CONFIG_SYS_GRLIB_GAISLER_SDCTRL2
struct mctrl_setup gaisler_sdctrl2_cfg = {
	.reg_mask = 0x1,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_SDCTRL2_CTRL,
		},
	}
};
#endif
#endif

#ifdef CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1
struct ahbmctrl_setup gaisler_ddr2spa1_cfg = {
	.ahb_mbar_no = 1,
	.reg_mask = 0xd,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1_CFG1,
		},
		{ 0x00000000, 0},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1_CFG3,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1_CFG4,
		},
	}
};
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDR2SPA2
struct ahbmctrl_setup gaisler_ddr2spa2_cfg = {
	.ahb_mbar_no = 1,
	.reg_mask = 0xd,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA2_CFG1,
		},
		{ 0x00000000, 0},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA2_CFG3,
		},
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDR2SPA2_CFG4,
		},
	}
};
#endif
#endif

#ifdef CONFIG_SYS_GRLIB_GAISLER_DDRSPA1
struct ahbmctrl_setup gaisler_ddrspa1_cfg = {
	.ahb_mbar_no = 1,
	.reg_mask = 0x1,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDRSPA1_CTRL,
		},
	}
};
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDRSPA2
struct ahbmctrl_setup gaisler_ddrspa2_cfg = {
	.ahb_mbar_no = 1,
	.reg_mask = 0x1,
	.regs = {
		{
			.mask = 0x00000000,
			.value = CONFIG_SYS_GRLIB_GAISLER_DDRSPA2_CTRL,
		},
	}
};
#endif
#endif

struct grlib_mctrl_handler grlib_mctrl_handlers[] = {
/* ESA MCTRL (PROM/FLASH/IO/SRAM/SDRAM) */
#ifdef CONFIG_SYS_GRLIB_ESA_MCTRL1
	{DEV_APB_SLV, 0, MH_UNUSED, AMBA_PNP_ID(VENDOR_ESA, ESA_MCTRL),
	_nomem_mctrl_init, (void *)&esa_mctrl1_cfg},
#ifdef CONFIG_SYS_GRLIB_ESA_MCTRL2
	{DEV_APB_SLV, 1, MH_UNUSED, AMBA_PNP_ID(VENDOR_ESA, ESA_MCTRL),
	_nomem_mctrl_init, (void *)&esa_mctrl2_cfg},
#endif
#endif

/* GAISLER Fault Tolerant Memory controller (PROM/FLASH/IO/SRAM/SDRAM) */
#ifdef CONFIG_SYS_GRLIB_GAISLER_FTMCTRL1
	{DEV_APB_SLV, 0, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_FTMCTRL),
	_nomem_mctrl_init, (void *)&gaisler_ftmctrl1_cfg},
#ifdef CONFIG_SYS_GRLIB_GAISLER_FTMCTRL2
	{DEV_APB_SLV, 1, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_FTMCTRL),
	_nomem_mctrl_init, (void *)&gaisler_ftmctrl2_cfg},
#endif
#endif

/* GAISLER SDRAM-only Memory controller (SDRAM) */
#ifdef CONFIG_SYS_GRLIB_GAISLER_SDCTRL1
	{DEV_APB_SLV, 0, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_SDCTRL),
	_nomem_mctrl_init, (void *)&gaisler_sdctrl1_cfg},
#ifdef CONFIG_SYS_GRLIB_GAISLER_SDCTRL2
	{DEV_APB_SLV, 1, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_SDCTRL),
	_nomem_mctrl_init, (void *)&gaisler_sdctrl2_cfg},
#endif
#endif

/* GAISLER DDR Memory controller (DDR) */
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDRSPA1
	{DEV_AHB_SLV, 0, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_DDRSP),
	_nomem_ahbmctrl_init, (void *)&gaisler_ddrspa1_cfg},
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDRSPA2
	{DEV_AHB_SLV, 1, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_DDRSP),
	_nomem_ahbmctrl_init, (void *)&gaisler_ddrspa2_cfg},
#endif
#endif

/* GAISLER DDR2 Memory controller (DDR2) */
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDR2SPA1
	{DEV_AHB_SLV, 0, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_DDR2SP),
	_nomem_ahbmctrl_init, (void *)&gaisler_ddr2spa1_cfg},
#ifdef CONFIG_SYS_GRLIB_GAISLER_DDR2SPA2
	{DEV_AHB_SLV, 1, MH_UNUSED, AMBA_PNP_ID(VENDOR_GAISLER, GAISLER_DDR2SP),
	_nomem_ahbmctrl_init, (void *)&gaisler_ddr2spa2_cfg},
#endif
#endif

	/* Mark end */
	MH_END
};

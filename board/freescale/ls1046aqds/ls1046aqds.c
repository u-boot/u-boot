// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2019-2021 NXP
 */

#include <config.h>
#include <clock_legacy.h>
#include <i2c.h>
#include <fdt_support.h>
#include <fsl_ddr_sdram.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/fdt.h>
#include <asm/arch/mmu.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <ahci.h>
#include <hwconfig.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <spl.h>
#include "../common/i2c_mux.h"

#include "../common/vid.h"
#include "../common/qixis.h"
#include "ls1046aqds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TFABOOT
struct ifc_regs ifc_cfg_nor_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nor0",
		CFG_SYS_NOR0_CSPR,
		CFG_SYS_NOR0_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},

	},
	{
		"nor1",
		CFG_SYS_NOR1_CSPR,
		CFG_SYS_NOR1_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"nand",
		CFG_SYS_NAND_CSPR,
		CFG_SYS_NAND_CSPR_EXT,
		CFG_SYS_NAND_AMASK,
		CFG_SYS_NAND_CSOR,
		{
			CFG_SYS_NAND_FTIM0,
			CFG_SYS_NAND_FTIM1,
			CFG_SYS_NAND_FTIM2,
			CFG_SYS_NAND_FTIM3
		},
	},
	{
		"fpga",
		CFG_SYS_FPGA_CSPR,
		CFG_SYS_FPGA_CSPR_EXT,
		CFG_SYS_FPGA_AMASK,
		CFG_SYS_FPGA_CSOR,
		{
			CFG_SYS_FPGA_FTIM0,
			CFG_SYS_FPGA_FTIM1,
			CFG_SYS_FPGA_FTIM2,
			CFG_SYS_FPGA_FTIM3
		},
	}
};

struct ifc_regs ifc_cfg_nand_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nand",
		CFG_SYS_NAND_CSPR,
		CFG_SYS_NAND_CSPR_EXT,
		CFG_SYS_NAND_AMASK,
		CFG_SYS_NAND_CSOR,
		{
			CFG_SYS_NAND_FTIM0,
			CFG_SYS_NAND_FTIM1,
			CFG_SYS_NAND_FTIM2,
			CFG_SYS_NAND_FTIM3
		},
	},
	{
		"nor0",
		CFG_SYS_NOR0_CSPR,
		CFG_SYS_NOR0_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"nor1",
		CFG_SYS_NOR1_CSPR,
		CFG_SYS_NOR1_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"fpga",
		CFG_SYS_FPGA_CSPR,
		CFG_SYS_FPGA_CSPR_EXT,
		CFG_SYS_FPGA_AMASK,
		CFG_SYS_FPGA_CSOR,
		{
			CFG_SYS_FPGA_FTIM0,
			CFG_SYS_FPGA_FTIM1,
			CFG_SYS_FPGA_FTIM2,
			CFG_SYS_FPGA_FTIM3
		},
	}
};

void ifc_cfg_boot_info(struct ifc_regs_info *regs_info)
{
	enum boot_src src = get_boot_src();

	if (src == BOOT_SOURCE_IFC_NAND)
		regs_info->regs = ifc_cfg_nand_boot;
	else
		regs_info->regs = ifc_cfg_nor_boot;
	regs_info->cs_size = CONFIG_SYS_FSL_IFC_BANK_COUNT;
}

#endif

enum {
	MUX_TYPE_GPIO,
};

int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	char buf[64];
#ifndef CONFIG_SD_BOOT
	u8 sw;
#endif

	puts("Board: LS1046AQDS, boot from ");

#ifdef CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	else {
#endif

#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#else
	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("PromJet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else if (sw == 0xF)
		printf("QSPI\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);
#endif

#ifdef CONFIG_TFABOOT
	}
#endif
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x\n",
	       QIXIS_READ(id), QIXIS_READ(arch));

	printf("FPGA:  v%d (%s), build %d\n",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());

	return 0;
}

bool if_board_diff_clk(void)
{
	u8 diff_conf = QIXIS_READ(brdcfg[11]);

	return diff_conf & 0x40;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0f) {
	case QIXIS_SYSCLK_64:
		return 64000000;
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}

	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	if (if_board_diff_clk())
		return get_board_sys_clk();
	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}

	return 66666666;
}

#ifdef CONFIG_LPUART
u32 get_lpuart_clk(void)
{
	return gd->bus_clk;
}
#endif

int dram_init(void)
{
	/*
	 * When resuming from deep sleep, the I2C channel may not be
	 * in the default channel. So, switch to the default channel
	 * before accessing DDR SPD.
	 *
	 * PCA9547 mount on I2C1 bus
	 */
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
	fsl_initdram();
#if (!defined(CONFIG_SPL) && !defined(CONFIG_TFABOOT)) || \
	defined(CONFIG_XPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}

int i2c_multiplexer_select_vid_channel(u8 channel)
{
	return select_i2c_ch_pca9547(channel, 0);
}

int board_early_init_f(void)
{
	u32 __iomem *cntcr = (u32 *)CFG_SYS_FSL_TIMER_ADDR;
#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CFG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;
#endif
#ifdef CONFIG_LPUART
	u8 uart;
#endif

	/*
	 * Enable secure system counter for timer
	 */
	out_le32(cntcr, 0x1);

#if defined(CONFIG_SYS_I2C_EARLY_INIT)
	i2c_early_init_f();
#endif
	fsl_lsch2_early_init_f();

#ifdef CONFIG_HAS_FSL_XHCI_USB
	out_be32(&scfg->rcwpmuxcr0, 0x3333);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
	usb_pwrfault = (SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB3_SHIFT) |
			(SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB2_SHIFT) |
			(SCFG_USBPWRFAULT_SHARED <<
			SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif

#ifdef CONFIG_LPUART
	/* We use lpuart0 as system console */
	uart = QIXIS_READ(brdcfg[14]);
	uart &= ~CFG_UART_MUX_MASK;
	uart |= CFG_LPUART_EN << CFG_UART_MUX_SHIFT;
	QIXIS_WRITE(brdcfg[14], uart);
#endif

	return 0;
}

#ifdef CONFIG_FSL_DEEP_SLEEP
/* determine if it is a warm boot */
bool is_warm_boot(void)
{
#define DCFG_CCSR_CRSTSR_WDRFR	(1 << 3)
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	if (in_be32(&gur->crstsr) & DCFG_CCSR_CRSTSR_WDRFR)
		return 1;

	return 0;
}
#endif

int config_board_mux(int ctrl_type)
{
	u8 reg14;

	reg14 = QIXIS_READ(brdcfg[14]);

	switch (ctrl_type) {
	case MUX_TYPE_GPIO:
		reg14 = (reg14 & (~0x6)) | 0x2;
		break;
	default:
		puts("Unsupported mux interface type\n");
		return -1;
	}

	QIXIS_WRITE(brdcfg[14], reg14);

	return 0;
}

int config_serdes_mux(void)
{
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	if (hwconfig("gpio"))
		config_board_mux(MUX_TYPE_GPIO);

	return 0;
}
#endif

int board_init(void)
{
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);

#ifdef CFG_SYS_FSL_SERDES
	config_serdes_mux();
#endif

	if (adjust_vdd(0))
		printf("Warning: Adjusting core voltage failed.\n");

#ifdef CONFIG_NXP_ESBC
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	u8 reg;

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_FMAN_ENET
	fdt_fixup_board_enet(blob);
#endif

	fdt_fixup_icid(blob);

	reg = QIXIS_READ(brdcfg[0]);
	reg = (reg & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	/* Disable IFC if QSPI is enabled */
	if (reg == 0xF)
		do_fixup_by_compat(blob, "fsl,ifc",
				   "status", "disabled", 8 + 1, 1);

	return 0;
}
#endif

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

#if defined(CONFIG_TFABOOT) && defined(CONFIG_ENV_IS_IN_SPI_FLASH)
void *env_sf_get_env_addr(void)
{
	return (void *)(CFG_SYS_FSL_QSPI_BASE + CONFIG_ENV_OFFSET);
}
#endif

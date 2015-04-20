/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <ahci.h>
#include <scsi.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
	writel(val, &crlapb_base->timestamp_ref_ctrl);

	/* Program freq register in System counter and enable system counter */
	writel(gd->cpu_clk, &iou_scntr->base_frequency_id_register);
	writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG |
	       ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
	       &iou_scntr->counter_control_register);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int timer_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_SCSI_AHCI_PLAT
void scsi_init(void)
{
	ahci_init(ZYNQMP_SATA_BASEADDR);
	scsi_scan(1);
}
#endif

int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM2)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR2,
						CONFIG_ZYNQ_GEM_PHY_ADDR2, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM3)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR3,
						CONFIG_ZYNQ_GEM_PHY_ADDR3, 0);
# endif
#endif
	return ret;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;
	u32 ver = zynqmp_get_silicon_version();

	if (ver != ZYNQMP_CSU_VERSION_VELOCE) {
#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
		ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
		ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	}

	return ret;
}
#endif

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		setenv("baudrate", "4800");
		setenv("bootcmd", "run veloce");
	case ZYNQMP_CSU_VERSION_EP108:
		setenv("serverip", "10.10.70.101");
		setenv("ipaddr", "10.10.71.100");
		setenv("partid", "auto");
		break;
	case ZYNQMP_CSU_VERSION_QEMU:
	default:
		setenv("serverip", "10.0.2.2");
		setenv("ipaddr", "10.0.2.15");
		setenv("partid", "0");
	}

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	switch (bootmode) {
	case JTAG_MODE:
		setenv("modeboot", "netboot");
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		setenv("modeboot", "qspiboot");
		break;
	case SD_MODE:
	case EMMC_MODE:
		setenv("modeboot", "sdboot");
		break;
	case NAND_MODE:
		setenv("modeboot", "nandboot");
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	return 0;
}

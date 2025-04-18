// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <stdlib.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <hang.h>
#include <log.h>
#include <ram.h>
#include <reset.h>
#include <wait_bit.h>
#include <wdt.h>
#include <linux/bitfield.h>
#include <linux/sizes.h>
#include <asm/arch/firewall.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "iossm_mailbox.h"
#include "sdram_soc64.h"

DECLARE_GLOBAL_DATA_PTR;

/* MPFE NOC registers */
#define F2SDRAM_SIDEBAND_FLAGOUTSET0	0x50
#define F2SDRAM_SIDEBAND_FLAGOUTSTATUS0	0x58
#define SIDEBANDMGR_FLAGOUTSET0_REG	SOCFPGA_F2SDRAM_MGR_ADDRESS +\
					F2SDRAM_SIDEBAND_FLAGOUTSET0
#define SIDEBANDMGR_FLAGOUTSTATUS0_REG	SOCFPGA_F2SDRAM_MGR_ADDRESS +\
					F2SDRAM_SIDEBAND_FLAGOUTSTATUS0
#define BOOT_SCRATCH_COLD3_REG	(socfpga_get_sysmgr_addr() +\
				 SYSMGR_SOC64_BOOT_SCRATCH_COLD3)
#define PORT_EMIF_CONFIG_OFFSET 4
#define EMIF_PLL_MASK	GENMASK(19, 16)

#define IO96B0_DUAL_PORT_MASK		BIT(0)
#define IO96B0_DUAL_EMIF_MASK		BIT(1)

#define FIREWALL_MPFE_SCR_IO96B0_REG		0x18000d00
#define FIREWALL_MPFE_SCR_IO96B1_REG		0x18000d04
#define FIREWALL_MPFE_NOC_CSR_REG		0x18000d08

#define MEMORY_BANK_MAX_COUNT 3

/* Reset type */
enum reset_type {
	POR_RESET,
	WARM_RESET,
	COLD_RESET,
	NCONFIG,
	JTAG_CONFIG,
	RSU_RECONFIG
};

phys_addr_t io96b_csr_reg_addr[] = {
	0x18400000, /* IO96B_0 CSR registers address */
	0x18800000  /* IO96B_1 CSR registers address */
};

struct dram_bank_info_s {
	phys_addr_t start;
	phys_size_t max_size;
};

struct dram_bank_info_s dram_bank_info[MEMORY_BANK_MAX_COUNT] = {
	{0x80000000, 0x80000000},	/* Memory Bank 0 */
	{0x880000000, 0x780000000},	/* Memory Bank 1 */
	{0x8800000000, 0x7800000000}	/* Memory Bank 2 */
};

static enum reset_type get_reset_type(u32 reg)
{
	return FIELD_GET(ALT_SYSMGR_SCRATCH_REG_3_DDR_RESET_TYPE_MASK, reg);
}

static void update_io96b_assigned_to_hps(bool dual_port_flag, bool dual_emif_flag)
{
	clrsetbits_le32(BOOT_SCRATCH_COLD3_REG,
			ALT_SYSMGR_SCRATCH_REG_3_DDR_PORT_EMIF_INFO_MASK,
			FIELD_PREP(ALT_SYSMGR_SCRATCH_REG_3_DDR_PORT_INFO_MASK, dual_port_flag) |
			FIELD_PREP(ALT_SYSMGR_SCRATCH_REG_3_DDR_EMIF_INFO_MASK, dual_emif_flag));

	debug("%s: update dual port dual emif info: 0x%x\n", __func__,
	      readl(BOOT_SCRATCH_COLD3_REG));
}

static void set_mpfe_config(void)
{
	/* Set mpfe_lite_intfcsel */
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_MPFE_CONFIG, BIT(2));

	/* Set mpfe_lite_active */
	setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_MPFE_CONFIG, BIT(8));

	debug("%s: mpfe_config: 0x%x\n", __func__,
	      readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_MPFE_CONFIG));
}

static bool is_ddr_init_hang(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_POR0);

	debug("%s: 0x%x\n", __func__, reg);

	if (reg & ALT_SYSMGR_SCRATCH_REG_POR_0_DDR_PROGRESS_MASK)
		return true;

	return false;
}

static void ddr_init_inprogress(bool start)
{
	if (start)
		setbits_le32(socfpga_get_sysmgr_addr() +
				SYSMGR_SOC64_BOOT_SCRATCH_POR0,
				ALT_SYSMGR_SCRATCH_REG_POR_0_DDR_PROGRESS_MASK);
	else
		clrbits_le32(socfpga_get_sysmgr_addr() +
				SYSMGR_SOC64_BOOT_SCRATCH_POR0,
				ALT_SYSMGR_SCRATCH_REG_POR_0_DDR_PROGRESS_MASK);
}

static void populate_ddr_handoff(struct udevice *dev, struct io96b_info *io96b_ctrl)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	int i;
	u32 len = SOC64_HANDOFF_SDRAM_LEN;
	u32 handoff_table[len];

	/* Read handoff for DDR configuration */
	socfpga_handoff_read((void *)SOC64_HANDOFF_SDRAM, handoff_table, len);

	/* Read handoff - dual port */
	plat->dualport = FIELD_GET(IO96B0_DUAL_PORT_MASK, handoff_table[PORT_EMIF_CONFIG_OFFSET]);
	debug("%s: dualport from handoff: 0x%x\n", __func__, plat->dualport);

	if (plat->dualport)
		io96b_ctrl->num_port = 2;
	else
		io96b_ctrl->num_port = 1;

	/* Read handoff - dual EMIF */
	plat->dualemif = FIELD_GET(IO96B0_DUAL_EMIF_MASK, handoff_table[PORT_EMIF_CONFIG_OFFSET]);
	debug("%s: dualemif from handoff: 0x%x\n", __func__, plat->dualemif);

	if (plat->dualemif)
		io96b_ctrl->num_instance = 2;
	else
		io96b_ctrl->num_instance = 1;

	io96b_ctrl->io96b_pll = FIELD_GET(EMIF_PLL_MASK,
					  handoff_table[PORT_EMIF_CONFIG_OFFSET]);
	debug("%s: io96b enabled pll from handoff: 0x%x\n", __func__, io96b_ctrl->io96b_pll);

	update_io96b_assigned_to_hps(plat->dualport, plat->dualemif);

	/* Assign IO96B CSR base address if it is valid */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		io96b_ctrl->io96b[i].io96b_csr_addr = io96b_csr_reg_addr[i];
		debug("%s: IO96B 0x%llx CSR enabled\n", __func__,
		      io96b_ctrl->io96b[i].io96b_csr_addr);
	}
}

static void config_mpfe_sideband_mgr(struct udevice *dev)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);

	/* Dual port setting */
	if (plat->dualport)
		setbits_le32(SIDEBANDMGR_FLAGOUTSET0_REG, BIT(4));

	/* Dual EMIF setting */
	if (plat->dualemif) {
		set_mpfe_config();
		setbits_le32(SIDEBANDMGR_FLAGOUTSET0_REG, BIT(5));
	}

	debug("%s: SIDEBANDMGR_FLAGOUTSTATUS0: 0x%x\n", __func__,
	      readl(SIDEBANDMGR_FLAGOUTSTATUS0_REG));
}

static void config_ccu_mgr(struct udevice *dev)
{
	int ret = 0;
	struct altera_sdram_plat *plat = dev_get_plat(dev);

	if (plat->dualport || plat->dualemif) {
		debug("%s: config interleaving on ccu reg\n", __func__);
		ret = uclass_get_device_by_name(UCLASS_NOP,
						"socfpga-ccu-ddr-interleaving-on", &dev);
	} else {
		debug("%s: config interleaving off ccu reg\n", __func__);
		ret = uclass_get_device_by_name(UCLASS_NOP,
						"socfpga-ccu-ddr-interleaving-off", &dev);
	}

	if (ret) {
		printf("interleaving on/off ccu settings init failed: %d\n", ret);
		hang();
	}
}

static void config_firewall_mpfe_csr(struct udevice *dev)
{
	int ret = 0;

	debug("%s: config Firewall setting for MPFE CSR\n", __func__);
	ret = uclass_get_device_by_name(UCLASS_NOP,
					"socfpga-noc-fw-mpfe-csr", &dev);

	if (ret) {
		printf("Firewall setting for MPFE CSR init failed: %d\n", ret);
		hang();
	}
}

static bool hps_ocram_dbe_status(void)
{
	u32 reg = readl(BOOT_SCRATCH_COLD3_REG);

	if (reg & ALT_SYSMGR_SCRATCH_REG_3_OCRAM_DBE_MASK)
		return true;

	return false;
}

int sdram_mmr_init_full(struct udevice *dev)
{
	int i, ret = 0;
	phys_size_t hw_size;
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	struct altera_sdram_priv *priv = dev_get_priv(dev);
	struct io96b_info *io96b_ctrl = malloc(sizeof(*io96b_ctrl));

	u32 reg = readl(BOOT_SCRATCH_COLD3_REG);
	enum reset_type reset_t = get_reset_type(reg);
	bool full_mem_init = false;

	/* DDR initialization progress status tracking */
	bool is_ddr_hang_be4_rst = is_ddr_init_hang();

	debug("DDR: SDRAM init in progress ...\n");
	ddr_init_inprogress(true);

	gd->bd = (struct bd_info *)malloc(sizeof(struct bd_info));
	memset(gd->bd, '\0', sizeof(struct bd_info));

	debug("DDR: Address MPFE 0x%llx\n", plat->mpfe_base_addr);

	/* Populating DDR handoff data */
	debug("DDR: Checking SDRAM configuration in progress ...\n");
	populate_ddr_handoff(dev, io96b_ctrl);

	/* Configuring MPFE sideband manager registers - dual port & dual emif */
	config_mpfe_sideband_mgr(dev);

	/* Configuring Interleave/Non-interleave ccu registers */
	config_ccu_mgr(dev);

	/* Configure if polling is needed for IO96B GEN PLL locked */
	io96b_ctrl->ckgen_lock = true;

	/* Ensure calibration status passing */
	init_mem_cal(io96b_ctrl);

	printf("DDR: Calibration success\n");

	/* Initiate IOSSM mailbox */
	io96b_mb_init(io96b_ctrl);

	/* DDR type, DDR size and ECC status) */
	ret = get_mem_technology(io96b_ctrl);
	if (ret) {
		printf("DDR: Failed to get DDR type\n");

		goto err;
	}

	ret = get_mem_width_info(io96b_ctrl);
	if (ret) {
		printf("DDR: Failed to get DDR size\n");

		goto err;
	}

	ret = ecc_enable_status(io96b_ctrl);
	if (ret) {
		printf("DDR: Failed to get ECC enabled status\n");

		goto err;
	}

	hw_size = io96b_ctrl->overall_size;

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, gd->bd);
	if (ret) {
		puts("DDR: Failed to decode memory node\n");
		ret = -ENXIO;

		goto err;
	}

	if (io96b_ctrl->inline_ecc)
		hw_size = CALC_INLINE_ECC_HW_SIZE(hw_size);

	if (gd->ram_size > hw_size) {
		printf("DDR: Warning: DRAM size from device tree (%lld MiB) exceeds\n",
		       gd->ram_size >> 20);
		printf(" the actual hardware capacity(%lld MiB). Memory configuration will be\n",
		       hw_size >> 20);
		printf(" adjusted to match the detected hardware size.\n");
		gd->ram_size = 0;
	}

	if (gd->ram_size > 0 && gd->ram_size != hw_size) {
		printf("DDR: Warning: DRAM size from device tree (%lld MiB)\n",
		       gd->ram_size >> 20);
		printf(" mismatch with hardware capacity(%lld MiB).\n",
		       hw_size >> 20);
	}

	if (gd->ram_size == 0 && hw_size > 0) {
		phys_size_t remaining_size, size_counter = 0;
		u8 config_dram_banks;

		if (CONFIG_NR_DRAM_BANKS > MEMORY_BANK_MAX_COUNT) {
			printf("DDR: Warning: CONFIG_NR_DRAM_BANKS(%d) is bigger than Max Memory Bank count(%d).\n",
			       CONFIG_NR_DRAM_BANKS, MEMORY_BANK_MAX_COUNT);
			printf(" Max Memory Bank count is in use instead of CONFIG_NR_DRAM_BANKS.\n");
			config_dram_banks = MEMORY_BANK_MAX_COUNT;
		} else {
			config_dram_banks = CONFIG_NR_DRAM_BANKS;
		}

		for (i = 0; i < config_dram_banks; i++) {
			remaining_size = hw_size - size_counter;
			if (remaining_size <= dram_bank_info[i].max_size) {
				gd->bd->bi_dram[i].start = dram_bank_info[i].start;
				gd->bd->bi_dram[i].size = remaining_size;
				debug("Memory bank[%d]  Starting address: 0x%llx  size: 0x%llx\n",
				      i, gd->bd->bi_dram[i].start, gd->bd->bi_dram[i].size);
				break;
			}

			gd->bd->bi_dram[i].start = dram_bank_info[i].start;
			gd->bd->bi_dram[i].size = dram_bank_info[i].max_size;

			debug("Memory bank[%d]  Starting address: 0x%llx  size: 0x%llx\n",
			      i, gd->bd->bi_dram[i].start, gd->bd->bi_dram[i].size);
			size_counter += gd->bd->bi_dram[i].size;
		}

		gd->ram_size = hw_size;
	}

	printf("%s: %lld MiB\n", io96b_ctrl->ddr_type, gd->ram_size >> 20);

	/* Is HPS cold or warm reset? If yes, Skip full memory initialization if ECC
	 *  enabled to preserve memory content
	 */
	if (io96b_ctrl->ecc_status) {
		if (ecc_interrupt_status(io96b_ctrl)) {
			if (CONFIG_IS_ENABLED(WDT)) {
				struct udevice *wdt;

				printf("DDR: ECC error recover start now\n");
				ret = uclass_first_device_err(UCLASS_WDT, &wdt);
				if (ret) {
					printf("DDR: Failed to trigger watchdog reset\n");
					hang();
				}

				wdt_expire_now(wdt, 0);
			}
			hang();
		}

		full_mem_init = hps_ocram_dbe_status() | is_ddr_hang_be4_rst;
		if (full_mem_init || !(reset_t == WARM_RESET || reset_t == COLD_RESET)) {
			ret = bist_mem_init_start(io96b_ctrl);
			if (ret) {
				printf("DDR: Failed to fully initialize DDR memory\n");

				goto err;
			}
		}

		printf("SDRAM-ECC: Initialized success\n");
	}

	sdram_size_check(gd->bd);
	printf("DDR: size check success\n");

	sdram_set_firewall(gd->bd);

	/* Firewall setting for MPFE CSR */
	config_firewall_mpfe_csr(dev);

	printf("DDR: firewall init success\n");

	priv->info.base = gd->bd->bi_dram[0].start;
	priv->info.size = gd->ram_size;

	/* Ending DDR driver initialization success tracking */
	ddr_init_inprogress(false);

	printf("DDR: init success\n");

err:
	free(io96b_ctrl);

	return ret;
}
